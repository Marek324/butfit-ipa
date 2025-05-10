try:
    import matplotlib.pyplot as plt
    from matplotlib.ticker import FuncFormatter
    import numpy as np
    import os
except ImportError:
    print("Required libraries are not installed. Please install matplotlib and numpy.")
    exit(1)

def load_data(data_dir: str, waves_range:range) -> dict[int, dict]:
    data = {}  
    
    for i in waves_range:
        waves_file = f"{data_dir}/{i}waves"
        try:
            with open(waves_file, 'r') as file:
                data[i] = {"ref": {"ns": [], "cpu": []}, "simd": {"ns": [], "cpu": []}}
                
                ref = True
                for line in file:
                    if ref:
                        line = line.strip().split()
                        data[i]["ref"]["ns"].append(int(line[0]))
                        data[i]["ref"]["cpu"].append(int(line[1]))
                        ref = False
                    else:
                        line = line.strip().split()
                        data[i]["simd"]["ns"].append(int(line[0]))
                        data[i]["simd"]["cpu"].append(int(line[1]))
                        ref = True
        except FileNotFoundError:
            print(f"File not found: {waves_file}")
            exit(1)
    
    return data


def readable_form(num: float) -> str:
    for unit in ['', 'K', 'M', 'G']:
        if abs(num) < 1000:
            return f"{num:.0f}{unit}"
        num /= 1000
    return f"{num:.1f}T"    


def plot_waves_cycles(data: dict[int, dict], graph_dir: str):
    wave_counts = sorted(data.keys())
    ref_means = [np.mean(data[w]["ref"]["cpu"]) for w in wave_counts]
    simd_means = [np.mean(data[w]["simd"]["cpu"]) for w in wave_counts]

    x = np.arange(len(wave_counts))  # the label locations
    width = 0.35  # width of the bars

    fig, ax = plt.subplots(figsize=(10, 6))
    bars1 = ax.bar(x - width/2, ref_means, width, label='Baseline', color='steelblue')
    bars2 = ax.bar(x + width/2, simd_means, width, label='AVX2 SIMD', color='seagreen')

    # Labels and formatting
    ax.set_xlabel('Number of Waves')
    ax.set_ylabel('CPU Cycles (average)')
    ax.set_title('CPU Cycles vs Number of Waves — Baseline vs AVX2')
    ax.set_xticks(x)
    ax.set_xticklabels(wave_counts)
    ax.legend()

    # Add value labels on bars
    for bar in bars1 + bars2:
        height = bar.get_height()
        ax.annotate(readable_form(height),
                    xy=(bar.get_x() + bar.get_width() / 2, height),
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=8)

    fig.tight_layout()
    plt.savefig(os.path.join(graph_dir, 'cpu_cycles_vs_waves.png'))
    plt.close()


def plot_speedup(data: dict[int, dict], graph_dir: str):
    wave_counts = sorted(data.keys())
    speedups = [
        np.mean(data[w]["ref"]["cpu"]) / np.mean(data[w]["simd"]["cpu"])
        for w in wave_counts
    ]

    x = np.arange(len(wave_counts))

    fig, ax = plt.subplots(figsize=(10, 6))
    bars = ax.bar(x, speedups, color='orange', width=0.6)

    ax.set_xlabel('Number of Waves')
    ax.set_ylabel('Speedup (x)')
    ax.set_title('Speedup vs Number of Waves (Baseline / AVX2)')
    ax.set_xticks(x)
    ax.set_xticklabels(wave_counts)

    # Add value labels
    for bar in bars:
        height = bar.get_height()
        ax.annotate(f'{height:.2f}x',
                    xy=(bar.get_x() + bar.get_width() / 2, height),
                    xytext=(0, 3),
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=8)

    fig.tight_layout()
    plt.savefig(os.path.join(graph_dir, 'speedup_vs_waves.png'))
    plt.close()


def plot_per_iteration_fluctuations(data: dict[int, dict], graph_dir: str):
    per_wave_dir = os.path.join(graph_dir, 'per_wave_fluctuations')
    os.makedirs(per_wave_dir, exist_ok=True)

    for w, wave_data in data.items():
        ref = wave_data['ref']['cpu']
        simd = wave_data['simd']['cpu']
        iterations = range(len(ref))

        fig, ax = plt.subplots(figsize=(12, 4))
        ax.plot(iterations, ref, label='Baseline', alpha=0.7)
        ax.plot(iterations, simd, label='AVX2', alpha=0.7)

        ax.set_xlabel('Iteration')
        ax.set_ylabel('CPU Cycles')
        ax.set_title(f'CPU Cycles per Iteration - {w} Wave{"s" if w > 1 else ""}')
        ax.legend()

        ax.yaxis.set_major_formatter(FuncFormatter(lambda x, _: readable_form(x)))

        fig.tight_layout()
        plt.savefig(os.path.join(per_wave_dir, f'{w}waves_fluctuation.png'))
        plt.close()

def plot_speedup_efficiency(data: dict[int, dict], graph_dir: str):
    wave_counts = sorted(data.keys())
    speedup_efficiency = [
        (np.mean(data[w]["ref"]["cpu"]) / np.mean(data[w]["simd"]["cpu"])) / w
        for w in wave_counts
    ]

    x = np.arange(len(wave_counts))

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(x, speedup_efficiency, marker='o', color='green', label='Speedup Efficiency')

    ax.set_xlabel('Number of Waves')
    ax.set_ylabel('Speedup Efficiency (Speedup / Waves)')
    ax.set_title('Speedup Efficiency vs Number of Waves')
    ax.set_xticks(x)
    ax.set_xticklabels(wave_counts)

    # Add value labels
    for i, value in enumerate(speedup_efficiency):
        ax.annotate(f'{value:.2f}', xy=(x[i], speedup_efficiency[i]),
                    xytext=(0, 5), textcoords="offset points", ha='center', va='bottom')

    fig.tight_layout()
    plt.savefig(os.path.join(graph_dir, 'speedup_efficiency_vs_waves.png'))
    plt.close()



def plot_cumulative_time_saved(data: dict[int, dict], graph_dir: str):
    wave_counts = sorted(data.keys())
    cumulative_savings = []

    for w in wave_counts:
        ref_cycles = np.array(data[w]["ref"]["cpu"])
        simd_cycles = np.array(data[w]["simd"]["cpu"])
        savings = np.cumsum(ref_cycles - simd_cycles)
        cumulative_savings.append(savings)

    x = np.arange(1, len(ref_cycles) + 1)

    fig, ax = plt.subplots(figsize=(10, 6))
    for i, savings in enumerate(cumulative_savings):
        ax.plot(x, savings, label=f'{wave_counts[i]} Waves')

    ax.set_xlabel('Iteration')
    ax.set_ylabel('Cumulative Time Saved (CPU Cycles)')
    ax.set_title('Cumulative Time Saved per Iteration for Different Wave Counts')
    ax.legend()

    fig.tight_layout()
    plt.savefig(os.path.join(graph_dir, 'cumulative_time_saved.png'))
    plt.close()



def plot_data(data: dict[int, dict], graph_dir: str):
    if not os.path.exists(graph_dir):
        os.makedirs(graph_dir)

    plot_waves_cycles(data, graph_dir)
    plot_speedup(data, graph_dir)
    plot_per_iteration_fluctuations(data, graph_dir)
    plot_speedup_efficiency(data, graph_dir)
    plot_cumulative_time_saved(data, graph_dir)


def main():
    data_dir = '../data'
    graph_dir = '../graphs'

    data = load_data(data_dir, range(1, 9))
    plot_data(data, graph_dir)

if __name__ == "__main__":
    main()
