#include "intro_sort.h"
#include "merge_sort.h"
#include "quick_sort.h"
#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// Opis przypadku testowego (scenariusza wejścia).
// -----------------------------------------------------------------------------
struct Scenario {
    std::string name;
    double sorted_prefix_ratio; // np. 0.25 -> 25% poczatkowych elementow posortowanych
    bool reverse_sorted;        // cala tablica odwrotnie posortowana
};

// -----------------------------------------------------------------------------
// Procedura pomocnicza: sprawdzenie porzadku w tablicy.
// -----------------------------------------------------------------------------
static bool is_sorted_with_order(const std::vector<int>& data, bool ascending) {
    if (data.empty()) return true;
    for (std::size_t i = 1; i < data.size(); i++) {
        if (ascending) {
            if (data[i - 1] > data[i]) return false;
        } else {
            if (data[i - 1] < data[i]) return false;
        }
    }
    return true;
}

static void print_array(const std::vector<int>& data) {
    for (std::size_t i = 0; i < data.size(); i++) {
        std::cout << std::setw(5) << data[i];
        if ((i + 1) % 10 == 0 || i + 1 == data.size()) {
            std::cout << '\n';
        }
    }
}

// Liczba z zakresu 0..max_value-1 (bez wartosci ujemnych) - tylko dla wizualizacji.
static int nice_small_value(RandomGenerator& rng, int max_value) {
    int v = rng.generate() % max_value;
    if (v < 0) v += max_value;
    return v;
}

// -----------------------------------------------------------------------------
// Wstepna weryfikacja poprawnosci wszystkich trzech algorytmow.
// Mala tablica: wizualnie. Duza tablica: programowo (rosnaco i malejaco).
// -----------------------------------------------------------------------------
static bool run_preliminary_verification(RandomGenerator& rng) {
    std::cout << "=== Wstepna weryfikacja poprawnosci sortowania ===\n";

    // 1) Mala tablica (20 elementow) - weryfikacja wizualna.
    std::vector<int> small(20);
    for (int& x : small) x = nice_small_value(rng, 1000);

    std::cout << "\n[Mala tablica] Dane wejsciowe:\n";
    print_array(small);

    std::vector<int> q = small, m = small, in = small;
    MergeSort merge_sorter;

    quick_sort(q.data(), 0, static_cast<int>(q.size()) - 1, true);
    merge_sorter.merge_sort(m.data(), 0, static_cast<int>(m.size()) - 1, true);
    intro_sort(in.data(), 0, static_cast<int>(in.size()) - 1, true);

    std::cout << "[Mala tablica] quick_sort (rosnaco):\n";  print_array(q);
    std::cout << "[Mala tablica] merge_sort (rosnaco):\n";  print_array(m);
    std::cout << "[Mala tablica] intro_sort (rosnaco):\n";  print_array(in);

    // 2) Wieksza tablica - sprawdzenie programowe (rosnaco i malejaco).
    const int big_size = 200000;
    std::vector<int> big(big_size);
    for (int& x : big) x = rng.generate();

    std::vector<int> q_asc = big, q_desc = big;
    std::vector<int> m_asc = big, m_desc = big;
    std::vector<int> i_asc = big, i_desc = big;

    quick_sort(q_asc.data(), 0, big_size - 1, true);
    quick_sort(q_desc.data(), 0, big_size - 1, false);
    merge_sorter.merge_sort(m_asc.data(), 0, big_size - 1, true);
    merge_sorter.merge_sort(m_desc.data(), 0, big_size - 1, false);
    intro_sort(i_asc.data(), 0, big_size - 1, true);
    intro_sort(i_desc.data(), 0, big_size - 1, false);

    const bool ok =
        is_sorted_with_order(q_asc, true)  && is_sorted_with_order(q_desc, false) &&
        is_sorted_with_order(m_asc, true)  && is_sorted_with_order(m_desc, false) &&
        is_sorted_with_order(i_asc, true)  && is_sorted_with_order(i_desc, false);

    if (!ok) {
        std::cerr << "Blad: weryfikacja poprawnosci nie powiodla sie.\n";
        return false;
    }
    std::cout << "[Wieksza tablica] Weryfikacja programowa: OK (rosnaco/malejaco).\n\n";
    return true;
}

// -----------------------------------------------------------------------------
// Budowa tablicy testowej zgodnie z zadanym scenariuszem.
// -----------------------------------------------------------------------------
static std::vector<int> build_test_array(int size, const Scenario& scenario, RandomGenerator& rng) {
    std::vector<int> data(size);
    for (int i = 0; i < size; i++) data[i] = rng.generate();

    if (scenario.reverse_sorted) {
        std::sort(data.begin(), data.end(), std::greater<int>());
        return data;
    }

    const int prefix_size = static_cast<int>(static_cast<double>(size) * scenario.sorted_prefix_ratio);
    if (prefix_size > 1) {
        std::sort(data.begin(), data.begin() + prefix_size);
    }
    return data;
}

// -----------------------------------------------------------------------------
// Pomiar czasu sortowania dla dowolnej funkcji sortujacej.
// Sortowana jest kopia tablicy (zeby mierzyc czysty czas algorytmu).
// -----------------------------------------------------------------------------
using SortFn = std::function<void(std::vector<int>&)>;

static double measure_ms(const std::vector<int>& input, const SortFn& sort_fn) {
    std::vector<int> data = input;
    const auto start = std::chrono::steady_clock::now();
    sort_fn(data);
    const auto end = std::chrono::steady_clock::now();

    if (!is_sorted_with_order(data, true)) {
        std::cerr << "Blad: algorytm nie posortowal tablicy poprawnie.\n";
        std::exit(1);
    }

    return std::chrono::duration<double, std::milli>(end - start).count();
}

// -----------------------------------------------------------------------------
// Benchmark: dla kazdego rozmiaru i scenariusza mierzymy 3 algorytmy na
// DOKLADNIE TEJ SAMEJ tablicy wejsciowej (uczciwe porownanie).
// -----------------------------------------------------------------------------
static void run_benchmark(RandomGenerator& rng, int trials_per_case,
                          const std::vector<int>& sizes,
                          std::ostream& csv_out) {
    const std::vector<Scenario> scenarios = {
        {"losowe_100",              0.0,   false},
        {"posortowane_25",          0.25,  false},
        {"posortowane_50",          0.50,  false},
        {"posortowane_75",          0.75,  false},
        {"posortowane_95",          0.95,  false},
        {"posortowane_99",          0.99,  false},
        {"posortowane_99_7",        0.997, false},
        {"odwrotnie_posortowane",   0.0,   true }
    };

    MergeSort merge_sorter;
    const std::vector<std::pair<std::string, SortFn>> algorithms = {
        {"quick_sort", [](std::vector<int>& v) {
            quick_sort(v.data(), 0, static_cast<int>(v.size()) - 1, true);
        }},
        {"merge_sort", [&merge_sorter](std::vector<int>& v) {
            merge_sorter.merge_sort(v.data(), 0, static_cast<int>(v.size()) - 1, true);
        }},
        {"intro_sort", [](std::vector<int>& v) {
            intro_sort(v.data(), 0, static_cast<int>(v.size()) - 1, true);
        }}
    };

    csv_out << "algorytm;rozmiar;przypadek;proby;sredni_ms;min_ms;max_ms\n";
    csv_out << std::fixed << std::setprecision(3);

    for (int size : sizes) {
        for (const Scenario& scenario : scenarios) {
            // Statystyki dla kazdego algorytmu.
            std::vector<double> sum_ms(algorithms.size(), 0.0);
            std::vector<double> min_ms(algorithms.size(), std::numeric_limits<double>::max());
            std::vector<double> max_ms(algorithms.size(), 0.0);

            for (int trial = 0; trial < trials_per_case; trial++) {
                // WAZNE: ta sama tablica wejsciowa dla wszystkich 3 algorytmow.
                const std::vector<int> input = build_test_array(size, scenario, rng);

                for (std::size_t a = 0; a < algorithms.size(); a++) {
                    const double ms = measure_ms(input, algorithms[a].second);
                    sum_ms[a] += ms;
                    min_ms[a] = std::min(min_ms[a], ms);
                    max_ms[a] = std::max(max_ms[a], ms);
                }
            }

            for (std::size_t a = 0; a < algorithms.size(); a++) {
                const double avg_ms = sum_ms[a] / trials_per_case;
                csv_out << algorithms[a].first << ";"
                        << size << ";"
                        << scenario.name << ";"
                        << trials_per_case << ";"
                        << avg_ms << ";"
                        << min_ms[a] << ";"
                        << max_ms[a] << "\n";
            }

            // Informacja postepu na stderr (nie psuje pliku CSV).
            std::cerr << "n=" << size << " " << scenario.name << " - OK\n";
        }
    }
}

// -----------------------------------------------------------------------------
// main + obsluga argumentow.
//   --verify / -v    tylko weryfikacja poprawnosci
//   --benchmark / -b tylko benchmark (bez weryfikacji)
//   --trials N       zmiana liczby prob (domyslnie 100)
//   --out PLIK       zapis CSV do pliku (domyslnie stdout)
//   --quick          szybki tryb: 10 prob, bez rozmiaru 1M
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    RandomGenerator rng;

    bool do_verify = true;
    bool do_benchmark = true;
    int trials = 100;
    std::string out_path;
    std::vector<int> sizes = {10000, 50000, 100000, 500000, 1000000};

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--verify" || a == "-v") {
            do_benchmark = false;
        } else if (a == "--benchmark" || a == "-b") {
            do_verify = false;
        } else if (a == "--trials" && i + 1 < argc) {
            trials = std::max(1, std::atoi(argv[++i]));
        } else if (a == "--out" && i + 1 < argc) {
            out_path = argv[++i];
        } else if (a == "--quick") {
            trials = 10;
            sizes = {10000, 50000, 100000, 500000};
        }
    }

    if (do_verify) {
        if (!run_preliminary_verification(rng)) return 1;
    }

    if (!do_benchmark) return 0;

    if (out_path.empty()) {
        run_benchmark(rng, trials, sizes, std::cout);
    } else {
        std::ofstream f(out_path);
        if (!f) {
            std::cerr << "Nie mozna otworzyc pliku: " << out_path << "\n";
            return 1;
        }
        run_benchmark(rng, trials, sizes, f);
        std::cerr << "Zapisano wyniki do: " << out_path << "\n";
    }

    return 0;
}
