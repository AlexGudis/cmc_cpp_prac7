import csv
import subprocess
import time
from input_generator import generate_input_csv
import re
import csv

def update_cooling_type(filename, new_cooling_type):
    """
    Заменяет тип охлаждения (3-й элемент в первой строке) в CSV-файле.
    Остальные данные сохраняются без изменений.
    """
    # Читаем весь CSV
    with open(filename, "r") as f:
        reader = list(csv.reader(f))
    
    # Меняем тип охлаждения (3-й элемент)
    reader[0][2] = new_cooling_type

    # Перезаписываем файл
    with open(filename, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(reader)

    print(f"Cooling type успешно заменён на '{new_cooling_type}' в файле {filename}")



def run_simulation(filename):
    """
    Запускает C++ программу с заданными параметрами и возвращает финальную стоимость и время выполнения.
    """
    total_cost = 0
    total_time = 0
    num_runs = 5

    for i in range(num_runs):
        print(f"Starting round {i}")
        
        result = subprocess.run(
            ["./main", "file", filename],
            capture_output=True,
            text=True
        )

        #print(result.stdout)
        # Найти строку с "Best solution found with cost:" и получить значение стоимости        
        match_k1 = re.search(r"Best solution found[^\n]*\n.*\(K1\)=([\d.]+)", result.stdout)
        k1_value = float(match_k1.group(1))

        match_time = re.search(r"Best solution found \(time ([\d.]+) s\)", result.stdout)
        exec_time = float(match_time.group(1))

        total_cost += k1_value
        total_time += exec_time

    average_cost = total_cost / num_runs
    average_time = total_time / num_runs

    return average_cost, average_time

def main():
    # Параметры для тестирования
    num_jobs_list = [500 * i for i in range(1, 21)]
    min_duration = 2
    max_duration = 20
    num_processors_list = [5 * i for i in range(1, 11)]
    cooling_methods = ["Boltzmann", "Cauchy", "Mixed"]

    # Открываем файл для записи результатов
    with open("results.csv", mode="w", newline="") as results_file:
        writer = csv.writer(results_file)
        writer.writerow(["num_jobs", "num_processors", "cooling_method", "k1", "execution_time"])

        for N in num_jobs_list:
            for M in num_processors_list:

                # Генерация файла с стандартными параметрами, меняться будет только тип понижения температуры 
                generate_input_csv(N, M, "Cauchy", min_duration, max_duration)

                for cooling_method in cooling_methods:
                    update_cooling_type("./input/input.csv", cooling_method)
                    print(f"Running with {N} jobs, {M} processors, and {cooling_method} cooling")
                    
                    average_criteria, average_time = run_simulation("./input/input.csv")
                    
                    writer.writerow([N, M, cooling_method, average_criteria, average_time])
                    print(f"Result: Jobs = {N}, Processors = {M}, Cooling = {cooling_method}, Average Criteria = {average_criteria}, Average Time = {average_time:.6f} seconds")

if __name__ == "__main__":
    main()