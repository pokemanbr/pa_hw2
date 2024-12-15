#include <iostream>
#include <vector>
#include <tuple>
#include <queue>
#include <atomic>
#include <tbb/tbb.h>
#include <tbb/parallel_for.h>

const int RUNS = 5;
const int MAX_THREADS = 4;
const std::string YES = "\033[1;32mYES\033[0m";
const std::string NO = "\033[1;31mNO\033[0m";

std::vector<int> bfs_seq(const std::vector<std::vector<int>> &graph, int start = 0) {
    std::vector<int> dist(graph.size(), -1);
    std::queue<int> q;
    q.push(start);
    dist[start] = 0;
    while (!q.empty()) {
        int node = q.front();
        q.pop();
        for (auto neighbor: graph[node]) {
            if (dist[neighbor] == -1) {
                dist[neighbor] = dist[node] + 1;
                q.push(neighbor);
            }
        }
    }
    return dist;
}

std::vector<int> filter(const std::vector<int> &input, std::function<bool(int)> predicate) {
    return tbb::parallel_reduce(
            tbb::blocked_range<size_t>(0, input.size()),
            std::vector<int>(),
            [&input, &predicate](const tbb::blocked_range <size_t> &range, std::vector<int> local_result) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    if (predicate(input[i])) {
                        local_result.push_back(input[i]);
                    }
                }
                return local_result;
            },
            [](std::vector<int> lhs, const std::vector<int> &rhs) {
                lhs.insert(lhs.end(), rhs.begin(), rhs.end());
                return lhs;
            }
    );
}


std::vector<int> scan(const std::vector<int> &input) {
    std::vector<int> result(input.size(), 0);
    tbb::parallel_scan(
            tbb::blocked_range<size_t>(0, input.size()),
            0,
            [&input, &result](const tbb::blocked_range <size_t> &range, int sum, bool is_final_scan) -> int {
                int local_sum = sum;
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    local_sum += input[i];
                    if (is_final_scan) {
                        result[i] = local_sum;
                    }
                }
                return local_sum;
            },
            [](int left, int right) -> int {
                return left + right;
            }
    );
    return result;
}


std::vector<int> bfs_par(const std::vector<std::vector<int>> &graph, int start = 0) {
    std::vector<std::atomic<int>> dist(graph.size());
    for (int i = 0; i < graph.size(); ++i) {
        dist[i] = -1;
    }
    std::vector<int> frontier = {start};
    dist[start] = 0;
    while (!frontier.empty()) {
        size_t current_size = frontier.size();
        std::vector<int> count(current_size);
        tbb::parallel_for(size_t(0), current_size, [&](size_t i) {
            count[i] = graph[frontier[i]].size();
        });
        count = scan(count);
        std::vector<int> next_frontier(count.back(), -1);
        tbb::parallel_for(size_t(0), current_size, [&](size_t i) {
            int node = frontier[i];
            int offset = 0;
            if (i != 0) {
                offset = count[i - 1];
            }
            int j = 0;
            for (auto neighbor: graph[node]) {
                int expected = -1;
                if (dist[neighbor].compare_exchange_strong(expected, dist[node] + 1)) {
                    next_frontier[offset + j] = neighbor;
                }
                ++j;
            }
        });
        frontier = filter(next_frontier, [](int x) { return x != -1; });
    }
    std::vector<int> result(dist.begin(), dist.end());
    return result;
}

std::vector<std::vector<int>> generate_cubic_graph(int side) {
    int size = side * side * side;
    std::vector<std::vector<int>> graph(size);
    for (int x = 0; x < side; ++x) {
        for (int y = 0; y < side; ++y) {
            for (int z = 0; z < side; ++z) {
                int idx = x * side * side + y * side + z;
                std::vector<std::tuple<int, int, int>> neighbors = {
                        std::make_tuple(x - 1, y, z),
                        std::make_tuple(x + 1, y, z),
                        std::make_tuple(x, y - 1, z),
                        std::make_tuple(x, y + 1, z),
                        std::make_tuple(x, y, z - 1),
                        std::make_tuple(x, y, z + 1)
                };
                for (auto &[nx, ny, nz]: neighbors) {
                    if (nx >= 0 && nx < side && ny >= 0 && ny < side && nz >= 0 && nz < side) {
                        graph[idx].push_back(nx * side * side + ny * side + nz);
                    }
                }
            }
        }
    }
    return graph;
}

std::vector<int> generate_dist_for_cubic_graph(int side) {
    int size = side * side * side;
    std::vector<int> dist(size);
    for (int x = 0; x < side; ++x) {
        for (int y = 0; y < side; ++y) {
            for (int z = 0; z < side; ++z) {
                int idx = x * side * side + y * side + z;
                dist[idx] = x + y + z;
            }
        }
    }
    return dist;
}

std::pair<bool, double> measure_bfs_performance(
        std::vector<std::vector<int>> graph,
        const std::function<std::vector<int>(std::vector<std::vector<int>> &, int)> &bfs_function,
        const std::vector<int> &expected_dist,
        int graph_start = 0
) {
    bool is_correct = true;
    double total_time = 0.0;
    for (int i = 0; i < RUNS; ++i) {
        auto start = std::chrono::steady_clock::now();
        auto dist = bfs_function(graph, graph_start);
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        total_time += elapsed.count();
        is_correct &= dist == expected_dist;
    }
    return {is_correct, total_time / RUNS};
}


int main() {
    tbb::task_arena arena(MAX_THREADS);

    auto graph_sides = {10, 50, 100, 250, 500};

    for (auto &side: graph_sides) {
        std::cout << "Graph side: " << side << std::endl;

        auto graph = generate_cubic_graph(side);
        auto expected_dist = generate_dist_for_cubic_graph(side);

        auto [correct_seq_bfs, seq_time] = measure_bfs_performance(graph, bfs_seq, expected_dist);
        std::cout << "Average sequential time: " << seq_time << " seconds" << std::endl;
        std::cout << "Sequential sort correct: " << (correct_seq_bfs ? YES : NO) << std::endl;

        auto [correct_par_bfs, par_time] = measure_bfs_performance(
                graph,
                [&](const std::vector<std::vector<int>> &graph, int start) -> std::vector<int> {
                    return arena.execute([&]() -> std::vector<int> {
                        return bfs_par(graph, start);
                    });
                },
                expected_dist
        );
        std::cout << "Average parallel time: " << par_time << " seconds" << std::endl;
        std::cout << "Parallel sort correct: " << (correct_par_bfs ? YES : NO) << std::endl;

        std::cout << "Boost: " << seq_time / par_time << std::endl << std::endl;
    }
    return 0;
}
