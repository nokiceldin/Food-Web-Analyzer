# Food-Web-Analyzer
C++ program that models predator–prey relationships and analyzes food web structures.

**Skills:** graph algorithms, adjacency lists, file parsing, data modeling  

## 🧩 Overview
This project builds a directed graph from species interaction data and computes key ecological insights:
- Identifies apex predators (no incoming edges)
- Finds basal species (no outgoing edges)
- Calculates trophic levels and connectivity measures
- Outputs a readable summary of the ecosystem structure

## ⚙️ Build & Run
```bash
make
./foodweb_main data/foodweb.txt

