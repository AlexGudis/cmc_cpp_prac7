import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os

def plot_heatmaps_by_cooling(csv_path: str, output_dir: str = "heatmaps"):
    """
    Строит тепловые карты по execution_time и k1
    для каждого типа охлаждения (cooling_method)
    и сохраняет их в виде отдельных PNG-файлов.
    """

    # Читаем CSV
    df = pd.read_csv(csv_path)

    # Проверяем наличие столбцов
    required_cols = {"num_jobs", "num_processors", "cooling_method", "k1", "execution_time"}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"CSV должен содержать столбцы: {', '.join(required_cols)}")

    # Создаём выходную директорию
    os.makedirs(output_dir, exist_ok=True)

    # Общие настройки оформления
    sns.set(style="whitegrid", font_scale=1.1)

    # Проходим по каждому типу охлаждения
    for method, group in df.groupby("cooling_method"):
        # Сводные таблицы
        pivot_time = group.pivot_table(
            index="num_processors",
            columns="num_jobs",
            values="execution_time",
            aggfunc="mean"
        ).sort_index(ascending=False)
        pivot_k1 = group.pivot_table(
            index="num_processors",
            columns="num_jobs",
            values="k1",
            aggfunc="mean"
        ).sort_index(ascending=False)

        # --- Карта по execution_time ---
        plt.figure(figsize=(8, 6))
        sns.heatmap(
            pivot_time,
            cmap="rocket_r",
            annot=True,
            fmt=".2f",
            cbar_kws={"label": "Execution time, s"}
        )
        plt.title(f"Algorithm execution time, s ({method})", fontsize=14)
        plt.xlabel("Number of tasks", fontsize=12)
        plt.ylabel("Number of processors", fontsize=12)
        plt.tight_layout()
        time_filename = os.path.join(output_dir, f"heatmap_{method}_time.png")
        plt.savefig(time_filename, dpi=300)
        plt.close()

        # --- Карта по k1 ---
        plt.figure(figsize=(8, 6))
        sns.heatmap(
            pivot_k1,
            cmap="crest",
            annot=True,
            fmt=".1f",
            cbar_kws={"label": "k1 value"}
        )
        plt.title(f"Parameter k1 distribution ({method})", fontsize=14)
        plt.xlabel("Number of tasks", fontsize=12)
        plt.ylabel("Number of processors", fontsize=12)
        plt.tight_layout()
        k1_filename = os.path.join(output_dir, f"heatmap_{method}_k1.png")
        plt.savefig(k1_filename, dpi=300)
        plt.close()

        print(f"✅ Saved heatmaps for {method}:")
        print(f"   {time_filename}")
        print(f"   {k1_filename}")

# Пример вызова:
plot_heatmaps_by_cooling("results.csv")
