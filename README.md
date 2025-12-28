
---

# Multi-Level Cache Simulator with LLC Prefetching

A high-performance C++ simulator designed to model a memory hierarchy consisting of an **L1 Cache**, an optional **L2 Cache**, and a **Prefetch Buffer** attached to the Last Level Cache (LLC). This tool evaluates cache performance and memory traffic using a fixed LRU replacement policy.

## Features

* **Two-Level Hierarchy**: Simulate L1 alone or an L1 + L2 system.
* **Fixed LRU Policy**: All evictions use the Least Recently Used (LRU) algorithm.
* **Stream Buffer Prefetching**: Implements  stream buffers, each capable of holding  blocks, to reduce LLC miss penalties.
* **Write Strategy**: Uses **Write-Back + Write-Allocate (WBWA)**.
* **Trace-Driven**: Analyzes standard hex-based memory traces.

---

## Installation & Building

1. **Clone the repository:**
```bash
git clone https://github.com/ash-olakangal/cache_simulator.git
cd cache_simulator

```


2. **Build the simulator:**
```bash
make

```


3. **Clean build files:**
```bash
make clean

```



---

## Usage

The simulator is executed using the following command structure. Note that if you wish to simulate only L1, set all L2 parameters to `0`.

### Command Syntax

```bash
./sim_cache <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <PREF_N> <PREF_M> <trace_file>

```

### Parameter Breakdown

| Parameter | Description |
| --- | --- |
| **BLOCKSIZE** | The size of a single cache block (in bytes). |
| **L1_SIZE** | Total capacity of the L1 cache (in bytes). |
| **L1_ASSOC** | Associativity of the L1 cache (e.g., `4` for 4-way). |
| **L2_SIZE** | Total capacity of the L2 cache (in bytes). Set to `0` for L1-only. |
| **L2_ASSOC** | Associativity of the L2 cache. Set to `0` for L1-only. |
| **PREF_N** | **Number of Prefetch Stream Buffers** allocated to the LLC. |
| **PREF_M** | **Number of Blocks** stored per stream buffer. |
| **trace_file** | Path to the input trace file (e.g., `traces/gcc_trace.txt`). |

---

## Understanding Prefetching (LLC Only)

The simulator implements a **Stream Buffer** prefetcher that sits between the Last Level Cache (LLC) and Main Memory.

* **Trigger:** When a request misses in the LLC, the prefetcher checks its stream buffers.
* **Buffer Hit:** If the requested block is in a stream buffer, it is moved to the LLC, and the buffer fetches the next  sequential blocks from memory.
* **Buffer Miss:** If the block is not in the buffers, a new stream buffer is allocated (using LRU) to start prefetching sequential blocks starting from the missed address.

---

## Example Configurations

### 1. Basic L1 Cache (No Prefetching)

```bash
./sim_cache 32 8192 4 0 0 0 0 traces/gcc_trace.txt

```

### 2. L1 + L2 Hierarchy with 4 Stream Buffers (8 blocks each)

```bash
./sim_cache 64 16384 8 131072 16 4 8 traces/perl_trace.txt

```

---

## Output & Metrics

The simulator outputs a detailed report containing:

1. **Cache Contents**: A snapshot of the final state of L1 and L2 (Tags and Dirty bits).
2. **Prefetcher State**: The final contents of the stream buffers.
3. **Simulation Statistics**:
* L1/L2 Read and Write counts.
* L1/L2 Miss rates.
* **Prefetcher Stats**: Number of prefetch requests and hits.
* **Total Memory Traffic**: Total number of blocks requested from Main Memory.



---

## 📝 Note on Policies

* **Replacement**: This simulator strictly uses **LRU (Least Recently Used)**. No manual policy selection is required.

---
