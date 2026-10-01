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
time. A coder cycles through compiling, debugging, and refactoring, then
immediately tries to compile again. If a coder goes longer than `time_to_burnout`
without starting a compile, they burn out and the session ends. A separate monitor
thread watches for burnout and for the moment every coder has compiled enough
times.

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
CFLAGS = -Wall -Wextra -Werror -pthread -I includes
```

`make clean`, `make fclean` and `make re` behave as usual.

### Usage

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

All 8 arguments are mandatory.

| Argument | Unit | Meaning |
| --- | --- | --- |
| `number_of_coders` | count | Number of coder threads, and of dongles on the table |
| `time_to_burnout` | ms | Max time a coder may go without starting a compile |
| `time_to_compile` | ms | How long a compile takes (both dongles held) |
| `time_to_debug` | ms | How long a coder spends debugging after compiling |
| `time_to_refactor` | ms | How long a coder spends refactoring; afterwards it immediately tries to compile again |
| `number_of_compiles_required` | count | The session ends successfully once every coder has compiled this many times |
| `dongle_cooldown` | ms | after being released, a dongle is unavailable until its cooldown has passed. |
| `scheduler` |	fifo / edf |	Arbitration policy used to decide who gets a contested dongle |

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

With `number_of_coders = 1` there is only one dongle on the table, and both of the
coder's hands (`left_hand`/`right_hand`) point at that same `t_dongle`. The coder
picks it up on the first `dongle_take` call; the second call sees
`dongle->holder == coder` (itself) and recognizes that this can never resolve — no
one else could ever release it — so it waits once via `dongle_wait` and gives up,
rather than looping forever. The coder is left holding its one dongle, never
compiles, and burns out once `time_to_burnout` has elapsed, exactly like a coder
that legitimately could not get a second dongle in time.


### Starvation prevention

Each dongle keeps a small wait queue (`t_queue`, at most 2 entries exactly the number of coders that can ever want the same dongle) and grants it fairly through `pick_next` (`src/heap.c`), using either `fifo` (earliest request wins) or `edf` (the coder closest to burning out wins).

### Cooldown handling

`dongle_release` records `released_time`; `grant_next` refuses to grant a dongle until `dongle_cooldown` milliseconds have passed since that release, so a dongle genuinely stays unavailable for the required window.

### Precise burnout detection

Burnout is detected by a dedicated monitor thread (`watching`) rather than by the
coders themselves. The monitor loops over every coder, compares
`codexion_clock() - coder->last_compiled` against `time_to_burnout` while holding
`session_mutex`, and sleeps 1 ms between sweeps. The poll interval is short
relative to the millisecond-scale timings, so a burnout is reported within about
1 ms of when it occurs.

`last_compiled` is written by the coder at the start of its compile and read by the
monitor, always under `session_mutex`, so the monitor never observes a torn or
stale value.

### Log serialization

All output passes through `log_mutex`, so two coders can never interleave their
writes mid-line, and the `burned out`/`finished` lines can never interleave with an
ordinary state-change line either. The stop flag itself (`session->someone_died`)
is a separate field guarded by `session_mutex`, not by `log_mutex`: `print_log`
(`src/utils.c`) checks it under `session_mutex` — once before doing any work, and
again right before the actual `printf` — and silently skips printing if it is set,
so state-change lines stop appearing once a burnout has been detected. `burned_out`
(`src/eye.c`) prints its line under `log_mutex` first and only then sets
`someone_died` under `session_mutex`, as two separate critical sections rather than
one combined one. In practice this means no *new* work starts after a burnout, but
when a program's stdout is piped or redirected (as opposed to a live terminal),
buffered `printf` output can still be flushed to the file *after* the unbuffered
`write()` call used for the burnout line, so a few already-in-flight lines with
timestamps from *before* the burnout can visually appear below it — a display
ordering artifact of stdio buffering, not a logic bug (the timestamp on each line
is still the true one).

## Thread synchronization mechanisms

### Shared state

| Resource | Shared between | Protected by |
| --- | --- | --- |
| Dongles | Neighbouring coders | One `pthread_mutex_t` per dongle |
| `stdout` | All coders and the monitor | `log_mutex` |
| `someone_died` flag | Monitor (writer), all coders (readers) | `session_mutex` |
| `last_compiled`, compile counters | Owning coder (writer), monitor (reader) | `session_mutex` |

### pthread_mutex_t

Each dongle's mutex does *not* double as the held resource itself — it only
protects that dongle's short-lived bookkeeping (`holder`, the wait queue) while
`dongle_take`, `dongle_release`, and `grant_next` (`src/dongle.c`, `src/heap.c`)
read or write it. Logical ownership is a separate field, `dongle->holder` (a
`t_coder *`, `NULL` when free), which a coder can hold across many other lock/
unlock cycles elsewhere while it is compiling. `dongle_take` enqueues the
requesting coder (`t_queue`, at most 2 entries) and loops — releasing the mutex,
sleeping a short slice, relocking it — until `grant_next` has set `holder` to that
exact coder, so a coder cannot compile without genuinely holding both dongles, but
the queueing and fairness (fifo/edf) are arbitrated by this code, not by the OS
mutex implementation.

`session_mutex` and `log_mutex` are kept separate on purpose. Timing data is read
by the monitor on every 1 ms sweep, while logging happens only on state changes;
putting both behind one mutex would make coders contend with the monitor's polling
for no reason.

Every read of shared state copies the value into a local variable and unlocks
before acting on it, so a mutex is never left held across a `continue` or `break`.
This is the actual check at the top of each coder's loop (`coder_workflow` in
`src/coder.c`):

```c
pthread_mutex_lock(&coder->session->session_mutex);
is_done = (coder->compile_count
		>= coder->session->parameter.number_of_compiles_required);
has_someone_died = coder->session->someone_died;
pthread_mutex_unlock(&coder->session->session_mutex);
if (is_done || has_someone_died)
	break ;
```

### Monitor to coder communication

Threads cannot be stopped from the outside — a thread ends only by returning from
its start routine. Termination is therefore cooperative. The monitor never tries to
kill anyone: `burned_out` (`src/eye.c`) sets `session->someone_died` under
`session_mutex` and `watching` then returns. Each coder tests `someone_died` at the
top of its loop, under the same `session_mutex`, and returns on its own. `main`
(via `start_end_session`) then joins the monitor and every coder, and only
afterwards destroys the mutexes and frees the session, so no thread can touch
memory that has already been released.

A coder that bails out mid-cycle unlocks every dongle it is holding before
returning. Skipping that would leave a neighbour blocked forever inside
`dongle_take`, unable to reach its own `someone_died` check, and `main` would hang
in `pthread_join`.

Note that `exit()` is deliberately not used to end the session. It would terminate
the process while other threads are still running, skipping the joins, the
`pthread_mutex_destroy` calls and the frees.

### Custom event implementation

No custom event mechanism was built. Each dongle does declare a
`pthread_cond_t condition`, and `dongle_release`/`grant_next`/`burned_out`
broadcast on it whenever a dongle changes state, but the waiting side
(`dongle_wait` in `src/dongle.c`) currently does not call `pthread_cond_wait` or
`pthread_cond_timedwait` on it — it waits by repeatedly unlocking the dongle's
mutex, sleeping a short slice (`usleep`), and relocking it. The broadcasts are
therefore currently inert; the actual wake-up mechanism is the polling loop, not
the condition variable.

## Resources

- `man 3 pthread_create`, `man 3 pthread_join`, `man 3 pthread_mutex_lock`, `man 3 pthread_cond_broadcast`, `man 2 gettimeofday`
- Dijkstra, E. W. — *Hierarchical Ordering of Sequential Processes* (1971), the origin of the dining philosophers problem
- Coffman, E. G., Elphick, M., Shoshani, A. — *System Deadlocks* (1971), the four conditions (used here for the deadlock-prevention discussion above)
- Butenhof, D. R. — *Programming with POSIX Threads*
- **Earliest Deadline First (EDF)** scheduling, used as one of the two dongle-arbitration policies (`scheduler` argument)

### AI usage

AI (Claude, via Claude Code) was used throughout this project's debugging,
with every explanation checked against the program's actual behavior
rather than taken on faith:

- Checking the cause of the Helgrind error, eventually claude did not find
  the alternative solution or fix for `pthread_cond_timewait`.
  But my collegue Kim found that I should replace with `usleep` and finally I managed to
  shut the error up.

