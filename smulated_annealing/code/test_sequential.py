import csv
import subprocess
import time
from input_generator import generate_input_csv
import re

def run_simulation(filename, num_processors, cooling_method):
    """
    Запускает C++ программу с заданными параметрами и возвращает финальную стоимость и время выполнения.
    """
    total_cost = 0
    total_time = 0
    num_runs = 5

    for _ in range(num_runs):
        
        start_time = time.time()
        result = subprocess.run(
            ["./main.out", "file", filename],
            capture_output=True,
            text=True
        )
        end_time = time.time()
        # Найти строку с "Best solution found with cost:" и получить значение стоимости
        
        output_lines = result.stdout
        match_k1 = re.search(r"Best solution found[^\n]*\n.*\(K1\)=([\d.]+)", output_lines)

        # Если стоимость не найдена, вернем None и сообщение об ошибке
        if match_k1 is None:
            print(f"Error in program output: {result.stdout}")
            return None, None

        k1_value = float(match_k1.group(1))
        total_cost += k1_value
        total_time += (end_time - start_time)

    average_cost = total_cost / num_runs
    average_time = total_time / num_runs

    return average_cost, average_time

def main():
    # Параметры для тестирования
    num_jobs_list = [256000, 128000, 64000, 32000, 16000]  # Различные значения количества работ
    min_duration = 1
    max_duration = 100
    num_processors_list = [200, 160, 80, 40]
    cooling_methods = ["boltzmann", "kosh", "SM"]

    # Открываем файл для записи результатов
    with open("results.csv", mode="w", newline="") as results_file:
        writer = csv.writer(results_file)
        writer.writerow(["num_jobs", "num_processors", "cooling_method", "final_cost", "execution_time"])

        # Запускаем программу с различными параметрами
        for N in num_jobs_list:
            # Генерация файла CSV для текущего значения num_jobs
            for M in num_processors_list:
                # Открываем файл, подменяем в нём куулинг метод и запускаем:
                # чтобы у одинаковых кулинг методов были одинаковые работы по длительностям


                for cooling_method in cooling_methods:
                    generate_input_csv(N, M, cooling_method, min_duration, max_duration)
                    print(f"Running with {N} jobs, {M} processors, and {cooling_method} cooling")
                    
                    average_criteria, average_time = run_simulation("jobs.csv", num_processors, cooling_method)
                    
                    if average_criteria is not None:
                        writer.writerow([N, M, cooling_method, average_criteria, average_time])
                        print(f"Result: Jobs = {N}, Processors = {M}, Cooling = {cooling_method}, Average Criteria = {average_criteria}, Average Time = {average_time:.2f} seconds")
                    else:
                        print("Error in simulation, skipping result")

if __name__ == "__main__":
    main()