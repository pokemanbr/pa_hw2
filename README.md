### Команда для запуска:

```bash
cmake -B build && cmake --build build && ./build/parallel_bfs
```

### Результат исполнения:

Graph side: 10\
Average sequential time: 0.000107834 seconds\
Sequential sort correct: <span style="color:green">YES</span>\
Average parallel time: 0.00179437 seconds\
Parallel sort correct: <span style="color:green">YES</span>\
Boost: 0.0600956

Graph side: 50\
Average sequential time: 0.0160998 seconds\
Sequential sort correct: <span style="color:green">YES</span>\
Average parallel time: 0.040755 seconds\
Parallel sort correct: <span style="color:green">YES</span>\
Boost: 0.395039

Graph side: 100\
Average sequential time: 0.186843 seconds\
Sequential sort correct: <span style="color:green">YES</span>\
Average parallel time: 0.27958 seconds\
Parallel sort correct: <span style="color:green">YES</span>\
Boost: 0.6683

Graph side: 250\
Average sequential time: 5.3758 seconds\
Sequential sort correct: <span style="color:green">YES</span>\
Average parallel time: 4.3452 seconds\
Parallel sort correct: <span style="color:green">YES</span>\
Boost: 1.23718

Graph side: 500\
Average sequential time: 61.837 seconds\
Sequential sort correct: <span style="color:green">YES</span>\
Average parallel time: 39.9199 seconds\
Parallel sort correct: <span style="color:green">YES</span>\
Boost: 1.54903
