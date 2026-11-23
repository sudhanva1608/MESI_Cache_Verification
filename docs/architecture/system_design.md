# Preliminary System Architecture

## High-Level Overview



## Component Architecture

### 1. Cache Model (src/cache/)
- **L1Cache**: Private per-core cache (configurable size, associativity, line size)
- **L2Cache**: Shared/private L2 cache with inclusive/exclusive policies
- **CacheLine**: Data structure representing a single cache line with MESI state
- **ReplacementPolicy**: Pluggable policies (LRU, FIFO, Random, PLRU)

**Key Interfaces**:
- read(address, core_id) -> (data, latency)
- write(address, data, core_id) -> latency
- snoop(address, transaction_type) -> snoop_response
- get_state(address) -> MESI_STATE

### 2. Coherence Protocol (src/coherence/)
- **MESIStateMachine**: Implements Modified/Exclusive/Shared/Invalid transitions
- **TransactionManager**: Handles BusRd, BusRdX, BusUpgr, Flush, FlushOpt
- **SnoopFilter**: Optional directory-based optimization
- **ProtocolChecker**: Validates protocol invariants at runtime

**MESI State Transitions** (preliminary):


### 3. Simulation Engine (src/simulator/)
- **CycleAccurateSimulator**: Event-driven simulation with cycle counting
- **CoreModel**: Simplified core issuing memory operations
- **Interconnect**: Bus or crossbar model with arbitration
- **EventQueue**: Priority queue for simulation events
- **StatisticsCollector**: Tracks hits, misses, coherence transactions, latency

**Configuration Parameters**:
- Number of cores (1-64)
- Cache hierarchy (L1 only, L1+L2, L1+L2+L3)
- Cache sizes, associativities, line sizes
- Interconnect type (bus, crossbar, mesh)
- Coherence protocol (MESI, MOESI, MESIF - extensible)

### 4. Test Generator (src/simulator/test_generator.py - planned)
- **LitmusTestGenerator**: Standard litmus tests (MP, SB, LB, IRIW, etc.)
- **RandomTestGenerator**: Random concurrent access patterns
- **DirectedTestGenerator**: Targeted tests for specific state transitions
- **ModelCheckTestGenerator**: Tests derived from model checking

### 5. Verification Engine (planned)
- **BoundedModelChecker**: SAT/SMT-based bounded verification
- **InvariantChecker**: Runtime assertion of coherence invariants
- **CounterexampleAnalyzer**: Minimizes and classifies failures
- **RegressionTracker**: Compares results across versions

## Data Flow



## Configuration System

Global configuration via config/simulator.yaml:


## Extensibility Points

1. **New coherence protocols**: Implement CoherenceProtocol interface
2. **New cache models**: Extend BaseCache class
3. **New interconnects**: Implement Interconnect interface
4. **New test generators**: Extend TestGenerator base class
5. **New verification backends**: Plug into VerificationEngine

## Assumptions and Constraints

- **Assumption**: Single-threaded simulation (event-driven), not cycle-parallel
- **Constraint**: Memory model assumes sequential consistency unless relaxed model added
- **Assumption**: No analog/timing effects (pure digital logic simulation)
- **Constraint**: Maximum 64 cores due to state space complexity

## Open Design Questions

1. Should simulator support relaxed memory models (TSO, RMO)?
2. Directory-based vs. snooping-based coherence - which to prioritize?
3. Integration with formal tools (murphi, nuXmv, CBMC)?
4. Trace format compatibility with existing tools (gem5, PIN)?
5. GPU acceleration for simulation?

---
*This is a preliminary design document. All architecture decisions are subject to change during implementation.*