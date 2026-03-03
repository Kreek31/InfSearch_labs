import matplotlib.pyplot as plt
import csv

ranks = []
freqs = []

with open("cipf.csv", encoding="utf-8") as f:
    reader = csv.reader(f)
    next(reader)
    for r, f in reader:
        ranks.append(int(r))
        freqs.append(int(f))

plt.figure(figsize=(8,6))
plt.loglog(ranks, freqs, label="Corpus")

cipf_line = [freqs[0] / r for r in ranks]
plt.loglog(ranks, cipf_line, linestyle="--", label="Cipf law")

plt.xlabel("Rank (log)")
plt.ylabel("Frequency (log)")
plt.legend()
plt.grid(True)

plt.savefig("cipf.png")
plt.show()