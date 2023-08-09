import csv
import gi
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
import scipy
from scipy.ndimage import gaussian_filter1d
from scipy.stats import pearsonr
import numpy as np

plt.style.use("seaborn-v0_8")
mpl.use("TkAgg")

df = pd.read_csv("Training Data.csv")
df = df.drop_duplicates(subset="generation", keep="first")
x_values = df.pop("generation")

def diff(v):
    return np.diff(v) / np.diff(x_values)

def plot_fitness():
    fig, ax = plt.subplots()


    ysmoothed = gaussian_filter1d(df["fitness"], sigma=2)
    dydx = diff(df["fitness"]) * 10
    ax.plot(x_values, ysmoothed)
    ax.plot(x_values[1:], dydx)

    ax.legend(["Fitness", "Derivative of Fitness (Amplified 10x)"])
    ax.set_xlabel("Generation Number")
    ax.set_ylabel("Fitness")
    ax.set_title("The Best Agent's Fitness Per Generation")
    
    plt.savefig("fitness")


def plot_weights():
    fitness = df.pop("fitness")
    diff_fitness = diff(fitness)
    CATEGORIES = {}

    for c in df.columns:
        parts = c.split(".")
        if not parts[0] in CATEGORIES:
            CATEGORIES[parts[0]] = []

        CATEGORIES[parts[0]].append(parts[1])


    # Create a plot for each category

    for c in CATEGORIES.keys():
        fig, ax = plt.subplots()

        parts = CATEGORIES[c]
        names = [f"{c}.{p}" for p in parts]
        cats = df[names]


        tot = 0

        ax.set_xlabel("Generation")
        # We only really care about the triple-stage weights
        if len(cats.columns) == 3:
            for cat in cats:

                vals = cats[cat]
                diff_v = abs(diff(vals))
                correlation = pearsonr(fitness, vals)
                print(f"Correlation for {cat} = {correlation.correlation}")
                tot += abs(correlation.correlation)
            ax.set_xlabel(f"Generation\nAverage Correlation with fitness: {(tot / len(cats.columns)):.2f}")

        ax.set_title(f"Weight values for the '{c.replace('_', ' ')}' parameter of the best agent while training")
        ax.set_ylabel("Evaluator Weight")
        ax.plot(x_values, cats)
        ax.legend(parts, loc="upper left")
        fig.savefig(f"Parameter {c}")
        # plt.show()
# print(plt.style.available)
plot_fitness()
plot_weights()