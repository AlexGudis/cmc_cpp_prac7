import subprocess
import csv
import re

# Функция для запуска программы с параметром и получения результата
def run_main_mult(filename, Nproc):
    # Запуск программы с параметром num_proc
    result = subprocess.run(
        ['./main_parallel', "file", filename, str(Nproc)],
        capture_output=True,
        text=True
    )

    #print(result.stdout)

    match_k1 = re.findall(r"Schedule\s*\(M=\d+,\s*N=\d+\):\s*\(K1\)=([\d.]+)", result.stdout)
    k1_value = float(match_k1[-1])  # Берём последнее (финальное) значение

    match_time = re.search(r"Общее время работы:\s*([\d.]+)\s*сек", result.stdout)
    exec_time = float(match_time.group(1))
    
    return exec_time, k1_value
    

# Запись результатов в CSV
def write_to_csv(filename, data):
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Запись заголовков
        writer.writerow(["num_proc", "avg_exec_time", "avg_final_cost"])
        # Запись данных
        writer.writerows(data)

# Основной блок программы
def main():
    data = []
    # Запуск тестирования 5 раз для каждого значения num_proc

    # Нужно сгенерировать какие-то данные, которые использовались и для последовательного, но менять число процессоров

    for Nproc in range(2, 15, 2):
        
        exec_times = []
        final_costs = []
        
        # Запуск программы 5 раз для получения среднего значения
        for _ in range(2):
            exec_time, final_cost = run_main_mult("input/input.csv", Nproc)
            exec_times.append(exec_time)
            final_costs.append(final_cost)

        # Рассчитываем среднее время выполнения и итоговую стоимость
        avg_exec_time = sum(exec_times) / len(exec_times)
        avg_final_cost = sum(final_costs) / len(final_costs)
        print(f"Nproc = {Nproc}, Time = {avg_exec_time}, K1 = {avg_final_cost}")

        # Добавляем результат в таблицу данных
        data.append([Nproc, avg_exec_time, avg_final_cost])

    # Запись всех результатов в CSV файл
    write_to_csv("results_parallel.csv", data)
    print("Результаты записаны в results_mult.csv")

if __name__ == "__main__":
    main()