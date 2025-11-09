#!/usr/bin/env python3
import random
import os

def generate_input_csv(N, M, cooling, minW, maxW, out_file="input.csv"):
    file_path = os.path.join("input", out_file)
    durations = [random.randint(minW, maxW) for _ in range(N)]

    with open(file_path, "w") as f:
        f.write(f"{N},{M},{cooling},{minW},{maxW}\n")
        f.write(",".join(map(str, durations)) + "\n")

if __name__ == "__main__":
    print("=== Генератор .csv для задачи расписания ===")
    N = int(input("Введите N (число работ): "))
    M = int(input("Введите M (число процессоров): "))
    cooling = input("Введите закон охлаждения (Cauchy / Boltzmann / Mixed): ")
    minW = int(input("Введите минимальную длительность работы: "))
    maxW = int(input("Введите максимальную длительность работы: "))

    input_dir = "input"
    if not os.path.exists(input_dir):
        os.makedirs(input_dir)
    out_file = input("Введите имя выходного файла (по умолчанию input.csv): ") or "input.csv"
    
    generate_input_csv(N, M, cooling, minW, maxW, out_file)

    print(f"\n✅ Файл '{out_file}' успешно создан!")
