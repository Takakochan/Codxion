_This project has been created as part of the 42 curriculum by tkunugi_


# Description
Race against time in this thrilling concurrency challenge! Orchestrate
multiple coders competing for limited USB dongles using POSIX threads, mutexes, and
smart scheduling—master resource synchronization before burnout strikes.

# CODEXION

This project has been created as part of the 42 curriculum by tkunugi

## Description

Race against time in this thrilling concurrency challenge! Orchestrate multiple
coders competing for limited USB dongles using POSIX threads, mutexes, and smart
scheduling — master resource synchronization before burnout strikes.

Each coder is a thread sitting at a shared table. To compile, a coder needs two
dongles — the one on their left and the one on their right — but there are only
as many dongles as there are coders, so neighbours can never compile at the same
time. A coder alternates between compiling, cooling down and thinking. If a coder
goes longer than `time_to_burnout` without starting a compile, they burn out and
the session ends. A separate monitor thread watches for burnout and for the moment
every coder has compiled enough times.

## Instructions

### Build

```bash
make        # builds the "codexion" binary
make re     # full rebuild
make clean  # removes object files
make fclean # removes object files and the binary
```

Compiler flags:

```
CFLAGS := -Wall -Wextra -Werror -g -pthread -I includes
```

`make clean`, `make fclean` and `make re` behave as usual.

### Usage

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_cooldown> [number_of_compiles]
```

| Argument | Unit | Meaning |
| --- | --- | --- |
| `number_of_coders` | count | Number of coder threads, and of dongles on the table |
| `time_to_burnout` | ms | Max time a coder may go without starting a compile |
| `time_to_compile` | ms | How long a compile takes (both dongles held) |
| `time_to_cooldown` | ms | How long a coder cools down after compiling |
| `number_of_compiles_required` | count | Optional. If given, the session ends once every coder has compiled this many times |
| `dongle_cooldown` | ms | after being released, a dongle is unavailable until its cooldown has passed. |
| `scheduler` |	fifo / edf |	Arbitration policy used to decide who gets a contested dongl |

Examples:

```bash
./codexion 5 800 200 100 100 7 0 fifo     # no cooldown, first-come-first-served
./codexion 5 800 200 100 100 7 50 edf     # cooldown, deadline-driven arbitration
./codexion 1 800 200 100 100 7 0 fifo     # single coder: only one dongle, burns out
```
### Input validation

Invalid input is rejected with an error message and a non-zero exit status, before any thread is created:

non-integer arguments, negative numbers, a scheduler value other than exactly fifo or edf, the wrong number of arguments


### Output

Every line is `timestamp_in_ms coder_id action`, with ids starting at 1:

```
1 2  has taken a dongle
2 2  has taken a dongle
2 2  is compiling
202 2  is debugging
203 1  has taken a dongle
204 1  has taken a dongle
204 1  is compiling
404 1  is debugging
```


## Blocking cases handled

### Deadlock prevention and Coffman's conditions

A deadlock in the classic Dining Philosophers problem happens when every coder grabs their left dongle and then waits forever for their right one, forming a cycle. Codexion avoids this by always acquiring the lower-numbered dongle first: in `coder_hand` (`src/coder.c`), whichever of `left_hand`/`right_hand` has the smaller `dongle_id` is requested first. Because every coder follows the same global order, the "hold one resource while waiting for the next one in a cycle" pattern can never form — this is the standard resource-ordering fix for one of Coffman's four deadlock conditions (circular wait).

### The single-coder case

With `number_of_coders = 1` there is only one dongle on the table. The coder picks
it up, waits for a second one that will never arrive, and burns out once
`time_to_burnout` has elapsed. This is handled explicitly rather than by letting
the coder block forever on `pthread_mutex_lock`, which would hang the program
instead of printing the burnout line and exiting.


### Starvation prevention

Each dongle keeps a small wait queue (`t_queue`, at most 2 entries exactly the number of coders that can ever want the same dongle) and grants it fairly through `pick_next` (`src/heap.c`), using either `fifo` (earliest request wins) or `edf` (the coder closest to burning out wins).

### Cooldown handling

`dongle_release` records `released_time`; `grant_next` refuses to grant a dongle until `dongle_cooldown` milliseconds have passed since that release, so a dongle genuinely stays unavailable for the required window.

### Precise burnout detection

Burnout is detected by a dedicated monitor thread (`eying_on`) rather than by the
coders themselves. The monitor loops over every coder, compares
`codexion_clock() - coder->last_compiled` against `time_to_burnout` while holding
`session_mutex`, and sleeps 1 ms between sweeps. The poll interval is short
relative to the millisecond-scale timings, so a burnout is reported within about
1 ms of when it occurs.

`last_compiled` is written by the coder at the start of its compile and read by the
monitor, always under `session_mutex`, so the monitor never observes a torn or
stale value.

### Log serialization

All output passes through a single `log_mutex`, so two coders can never interleave
their writes mid-line. The same mutex also guards the `stop` flag, and this is what
makes the last line reliable: the monitor sets `stop` and prints the terminal
message inside one critical section, while every ordinary log call checks `stop`
inside that same critical section before printing. A coder that was about to print
therefore either gets in before the terminal line or does not print at all — no
message can appear after `burned out` or `finished`.

## Thread synchronization mechanisms

### Shared state

| Resource | Shared between | Protected by |
| --- | --- | --- |
| Dongles | Neighbouring coders | One `pthread_mutex_t` per dongle |
| `stdout` | All coders and the monitor | `log_mutex` |
| `stop` flag | Monitor (writer), all coders (readers) | `log_mutex` |
| `last_compiled`, compile counters | Owning coder (writer), monitor (reader) | `session_mutex` |

### pthread_mutex_t

Dongles are plain mutexes; taking a dongle is `pthread_mutex_lock` on it and
putting it back is `pthread_mutex_unlock`. Because the mutex *is* the resource, a
coder cannot compile without genuinely holding both, and the kernel handles the
queueing of waiters.

`session_mutex` and `log_mutex` are kept separate on purpose. Timing data is read
by the monitor on every 1 ms sweep, while logging happens only on state changes;
putting both behind one mutex would make coders contend with the monitor's polling
for no reason.

Every read of shared state copies the value into a local variable and unlocks
before returning it, so a mutex is never left held across a `return`:

```c
int	should_stop(t_session *session)
{
	int	stop;

	pthread_mutex_lock(&session->log_mutex);
	stop = session->stop;
	pthread_mutex_unlock(&session->log_mutex);
	return (stop);
}
```

### Monitor to coder communication

Threads cannot be stopped from the outside — a thread ends only by returning from
its start routine. Termination is therefore cooperative. The monitor never tries to
kill anyone: it sets `stop` under `log_mutex` and returns. Each coder tests `stop`
at the top of its loop and returns on its own. `main` then joins the monitor and
every coder, and only afterwards destroys the mutexes and frees the session, so no
thread can touch memory that has already been released.

A coder that bails out mid-cycle unlocks every dongle it is holding before
returning. Skipping that would leave a neighbour blocked forever inside
`pthread_mutex_lock`, unable to reach its own `stop` check, and `main` would hang
in `pthread_join`.

Note that `exit()` is deliberately not used to end the session. It would terminate
the process while other threads are still running, skipping the joins, the
`pthread_mutex_destroy` calls and the frees.

### Custom event implementation

> **TODO:** same — document it if you built one, delete the section if you did not.

## Resources

> **TODO:** list what you actually used. Candidates:

- `man 3 pthread_create`, `man 3 pthread_join`, `man 3 pthread_mutex_lock`, `man 2 gettimeofday`
- Dijkstra, E. W. — *Hierarchical Ordering of Sequential Processes* (1971), the origin of the dining philosophers problem
- Coffman, E. G., Elphick, M., Shoshani, A. — *System Deadlocks* (1971), the four conditions
- Butenhof, D. R. — *Programming with POSIX Threads*
