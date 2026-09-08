#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define G 9.81
#define CANVAS_SIZE 10000.0 // Fixed Canvas Size D x D in meters
#define MAX_E 100
#define MAX_K 50

// Struct for Escort Ship
struct EscortShip {
    int id;
    int type; // 0: EA, 1: EB, 2: EC, 3: ED, 4: EE
    char typeName[30];
    double x, y;
    double minAngle;
    double maxAngle;
    double minVel;
    double maxVel;
    double impactPower;
    double maxRange;
    int isDestroyed;  // 1 if destroyed, 0 if active
    int hasAttacked;  // 1 if this E ship fired its shell
};

// Struct for Battleship
struct Battleship {
    char type; // 'U', 'M', 'R', 'S'
    char name[30];
    double x, y;
    double maxVel;
    double maxRange;
    double reloadTime;       // T_B^q: time between consecutive gun firings
    double cumulativeImpact; // Total damage taken
};

// Struct for Path Points
struct Point {
    double x, y;
};

// Calculate maximum projectile range
double getRange(double v) {
    return (v * v) / G;
}

// Distance formula
double getDistance(double x1, double y1, double x2, double y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Custom Strategy: Find the highest priority target for Battleship
// Threat Score = ImpactPower / Distance (higher threat = higher priority)
int selectTargetStrategy(struct Battleship B, struct EscortShip E[], int N) {
    int bestIndex = -1;
    double maxThreatScore = -1.0;

    for (int i = 0; i < N; i++) {
        if (!E[i].isDestroyed) {
            double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
            if (dist <= B.maxRange) {
                // Calculate threat score
                double threatScore = E[i].impactPower / (dist + 1.0);
                if (threatScore > maxThreatScore) {
                    maxThreatScore = threatScore;
                    bestIndex = i;
                }
            }
        }
    }
    return bestIndex; // Returns index of chosen target, or -1 if no target in range
}

int main() {
    srand(time(NULL));

    double D = CANVAS_SIZE;
    int N, K;

    printf("=== NAVAL BATTLE SIMULATOR (PART 2-A) ===\n");
    printf("Canvas Size: %.0fx%.0f meters\n\n", D, D);

    // 1. User Inputs
    printf("Enter number of escort ships (N): ");
    scanf("%d", &N);

    printf("Enter number of path points for Battleship (K): ");
    scanf("%d", &K);

    // 2. Battleship Setup
    struct Battleship B;
    printf("\nSelect Battleship Type (U = USS Iowa, M = MS King George, R = Richelieu, S = Sovetsky Soyuz): ");
    scanf(" %c", &B.type);

    if (B.type == 'M' || B.type == 'm') {
        sprintf(B.name, "MS King George V");
        B.reloadTime = 30.0; // 30 seconds reload time
    } else if (B.type == 'R' || B.type == 'r') {
        sprintf(B.name, "Richelieu");
        B.reloadTime = 32.0; // 32 seconds reload time
    } else if (B.type == 'S' || B.type == 's') {
        sprintf(B.name, "Sovetsky Soyuz-class");
        B.reloadTime = 35.0; // 35 seconds reload time
    } else {
        B.type = 'U';
        sprintf(B.name, "USS Iowa (BB-61)");
        B.reloadTime = 20.0; // 20 seconds reload time
    }

    B.maxVel = 400 + rand() % 400;
    B.maxRange = getRange(B.maxVel);
    B.cumulativeImpact = 0.0;

    // Generate K path points
    struct Point path[MAX_K];
    for (int i = 0; i < K; i++) {
        path[i].x = rand() % (int)D;
        path[i].y = rand() % (int)D;
    }

    // 3. Escort Ships Setup
    struct EscortShip E[MAX_E];

    for (int i = 0; i < N; i++) {
        E[i].id = i;
        E[i].x = rand() % (int)D;
        E[i].y = rand() % (int)D;
        E[i].type = rand() % 5;
        E[i].isDestroyed = 0;
        E[i].hasAttacked = 0;

        E[i].minAngle = 10 + rand() % 20;
        E[i].minVel = 50 + rand() % 50;

        if (E[i].type == 0) {
            sprintf(E[i].typeName, "1936A-class Destroyer");
            E[i].maxAngle = E[i].minAngle + 20;
            E[i].maxVel = 1.2 * B.maxVel;
            E[i].impactPower = 0.08;
        } else if (E[i].type == 1) {
            sprintf(E[i].typeName, "Gabbiano-class Corvette");
            E[i].maxAngle = E[i].minAngle + 30;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.06;
        } else if (E[i].type == 2) {
            sprintf(E[i].typeName, "Matsu-class Destroyer");
            E[i].maxAngle = E[i].minAngle + 25;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.07;
        } else if (E[i].type == 3) {
            sprintf(E[i].typeName, "F-class Escort Ship");
            E[i].maxAngle = E[i].minAngle + 50;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.05;
        } else {
            sprintf(E[i].typeName, "Japanese Kaibokan");
            E[i].maxAngle = E[i].minAngle + 70;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.04;
        }

        E[i].maxRange = getRange(E[i].maxVel);
    }

    // Save Initial Conditions File
    FILE *fInit = fopen("part2a_initial_conditions.txt", "w");
    if (fInit != NULL) {
        fprintf(fInit, "INITIAL BATTLEFIELD CONDITIONS (PART 2-A)\n");
        fprintf(fInit, "Battleship: %s | Firing Reload Time (T_B): %.1f seconds\n\n", B.name, B.reloadTime);
        fprintf(fInit, "ID\tType\t\t\t\tX\tY\tMaxVel\tImpactPower\tMaxRange\n");
        for (int i = 0; i < N; i++) {
            fprintf(fInit, "%d\t%-25s\t%.1f\t%.1f\t%.1f\t%.2f\t\t%.2f\n",
                    E[i].id, E[i].typeName, E[i].x, E[i].y, E[i].maxVel, E[i].impactPower, E[i].maxRange);
        }
        fclose(fInit);
    }

    // ==========================================
    // SIMULATION WITH RELOAD TIME & CUSTOM STRATEGY
    // ==========================================
    printf("\n--- RUNNING PART 2-A SIMULATION ---\n");
    FILE *fSim = fopen("part2a_simulation_results.txt", "w");

    int bSunk = 0;
    int sunkStep = -1;
    double totalTimeElapsed = 0.0;

    for (int k = 0; k < K; k++) {
        B.x = path[k].x;
        B.y = path[k].y;

        if (fSim != NULL) {
            fprintf(fSim, "\n--- Step %d at Position (%.1f, %.1f) | Time: %.1fs ---\n", k + 1, B.x, B.y, totalTimeElapsed);
        }

        // 1. Escort ships attempt to attack Battleship B
        for (int i = 0; i < N; i++) {
            if (!E[i].isDestroyed && !E[i].hasAttacked) {
                double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                if (dist <= E[i].maxRange) {
                    E[i].hasAttacked = 1;
                    B.cumulativeImpact += E[i].impactPower;

                    if (fSim != NULL) {
                        fprintf(fSim, "E-Ship ID %d attacked B! Added impact: %.2f (Total: %.2f)\n",
                                E[i].id, E[i].impactPower, B.cumulativeImpact);
                    }

                    if (B.cumulativeImpact >= 1.0) {
                        bSunk = 1;
                        sunkStep = k + 1;
                        if (fSim != NULL) {
                            fprintf(fSim, "Battleship %s SUNK at step %d!\n", B.name, k + 1);
                        }
                        break;
                    }
                }
            }
        }

        if (bSunk) break;

        // 2. Battleship uses strategy to pick targets in range and fire with reload time T_B^q
        if (fSim != NULL) {
            fprintf(fSim, "Battleship Attack Sequence at Step %d:\n", k + 1);
        }

        while (1) {
            int targetIdx = selectTargetStrategy(B, E, N);
            if (targetIdx == -1) break; // No targets left in attack range

            // Destroy selected target
            E[targetIdx].isDestroyed = 1;
            totalTimeElapsed += B.reloadTime; // Add reload time delay between firings

            if (fSim != NULL) {
                fprintf(fSim, " -> Targeted & Destroyed E-Ship ID %d (Type: %s) [Time: +%.1fs]\n",
                        E[targetIdx].id, E[targetIdx].typeName, B.reloadTime);
            }
        }
    }

    if (fSim != NULL) {
        fprintf(fSim, "\nFINAL SUMMARY (PART 2-A):\n");
        fprintf(fSim, "Battleship Status: %s\n", bSunk ? "SUNK" : "SURVIVED");
        fprintf(fSim, "Cumulative Impact Taken: %.2f (%.1f%%)\n", B.cumulativeImpact, B.cumulativeImpact * 100.0);
        fprintf(fSim, "Total Simulation Time: %.1f seconds\n", totalTimeElapsed);
        fclose(fSim);
    }

    if (bSunk) {
        printf("Simulation Result: Battleship SUNK at path point %d!\n", sunkStep);
    } else {
        printf("Simulation Result: Battleship SURVIVED! Cumulative Impact: %.2f (%.1f%%)\n",
               B.cumulativeImpact, B.cumulativeImpact * 100.0);
        printf("Total Simulation Firing Time: %.1f seconds\n", totalTimeElapsed);
    }

    printf("\nAll results saved to 'part2a_initial_conditions.txt' and 'part2a_simulation_results.txt'.\n");

    return 0;
}
