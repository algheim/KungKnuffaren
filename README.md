# Kungknuffaren

Chess bot written in C. Frontend written in python using pygame. Far from done.

Inspired by Sebastian Lague: https://www.youtube.com/watch?v=U4ogK0MIzqk

## Feautres
- Movegeneration using bitboards and precomputed attack tables.
- alpha beta pruning with move ordering.
- Quiescence search using delta pruning.
- Iterative deepening.
- Transposition table using zobrist hashing.
- Null move pruning.
- Killer moves.

## Running the Backend

1. `cd backend`
2. `make`
3. `bin/kungknuffaren`


## Running the frontend

1. `pip install -r requirements.txt`
2. `cd backend`
3. `make frontend_lib`
4. `cd ../frontend`
5. `python3 main.py`
