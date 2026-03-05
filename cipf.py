import matplotlib.pyplot as plt
import csv

ranks_mas = []
freqs_mas = []

with open("cipf.csv", encoding="utf-8") as f:
    reader = csv.reader(f)
    next(reader)
    for row in reader:
        rank = row[0]
        freq = row[1]
        ranks_mas .append(int(rank))
        freqs_mas .append(int(freq))

plt.figure(figsize=(8,6))
plt.loglog(ranks_mas , freqs_mas , label="Corpus")

cipf_line = [freqs_mas [0] / r for r in ranks_mas ]
plt.loglog(ranks_mas , cipf_line, linestyle="--", label="Cipf law")

plt.xlabel("Rank (log)")
plt.ylabel("Frequency (log)")
plt.legend()
plt.grid(True)

plt.savefig("cipf.png")
plt.show()