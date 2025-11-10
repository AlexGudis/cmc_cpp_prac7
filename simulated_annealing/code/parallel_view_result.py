import matplotlib.pyplot as plt
import csv

# Чтение данных из CSV
def load_csv(file_path):
    processes = []
    times = []
    costs = []
    
    with open(file_path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            processes.append(int(row["num_proc"]))
            times.append(float(row["avg_exec_time"]))
            costs.append(float(row["avg_final_cost"]))
    
    return processes, times, costs

# Построение графиков
def draw_charts(processes, times, costs):
    fig, axes = plt.subplots(2, 1, figsize=(10, 8), constrained_layout=True)

    # График времени выполнения
    axes[0].plot(processes, times, marker='s', linestyle='--', color='#FF5733', linewidth=2)
    axes[0].set_title("Execution Time vs Number of Processes", fontsize=14, fontweight='bold')
    axes[0].set_xlabel("Number of Processes")
    axes[0].set_ylabel("Execution Time (s)")
    axes[0].grid(True, linestyle=':', alpha=0.7)

    # График итоговой стоимости
    axes[1].plot(processes, costs, marker='^', linestyle='-.', color='#33A1FF', linewidth=2)
    axes[1].set_title("K1 value vs Number of Processes", fontsize=14, fontweight='bold')
    axes[1].set_xlabel("Number of Processes")
    axes[1].set_ylabel("K1 value")
    axes[1].grid(True, linestyle='--', alpha=0.7)

    # Сохранение графиков
    plt.savefig("./output/parallel_metrics.png", dpi=300)
    # plt.show()

# Основная функция
def main():
    file_path = "results_parallel.csv"
    processes, times, costs = load_csv(file_path)
    draw_charts(processes, times, costs)

if __name__ == "__main__":
    main()
