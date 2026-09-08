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
    double reloadTime;   // T_E^p: time between consecutive firings
    double nextFireTime; // Next timestamp when this E ship can fire
    double maxRange;
    int isDestroyed;     // 1 if destroyed, 0 if active
};

// Struct for Battleship
struct Battleship {
    char type; // 'U', 'M', 'R', 'S'
    char name[30];
    double x, y;
    double maxVel;
    double maxRange;
    double reloadTime;       // T_B^q: time between consecutive gun firings
    double nextFireTime;     // Next timestamp when Battleship can fire
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

// Update Escort Ship position moving toward Battleship B by velocity * dt
void updateEscortPosition(struct EscortShip *E, struct Battleship B, double dt) {
    if (E->isDestroyed) return;

    double dist = getDistance(E->x, E->y, B.x, B.y);
    if (dist > 1.0) { // Move toward B if not already at the same point
        double dirX = (B.x - E->x) / dist;
        double dirY = (B.y - E->y) / dist;

        E->x += dirX * E->maxVel * dt;
        E->y += dirY * E->maxVel * dt;
    }
}

// Custom Strategy: Find highest priority target for Battleship
// Threat Score = (ImpactPower / ReloadTime) / Distance
int selectTargetStrategy(struct Battleship B, struct EscortShip E[], int N) {
    int bestIndex = -1;
    double maxThreatScore = -1.0;

    for (int i = 0; i < N; i++) {
        if (!E[i].isDestroyed) {
            double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
            if (dist <= B.maxRange) {
                double threatScore = (E[i].impactPower / E[i].reloadTime) / (dist + 1.0);
                if (threatScore > maxThreatScore) {
                    maxThreatScore = threatScore;
                    bestIndex = i;
                }
            }
        }
    }
    return bestIndex;
}

int main() {
    srand(time(NULL));

    double D = CANVAS_SIZE;
    int N, K;

    printf("=== NAVAL BATTLE SIMULATOR (PART 2-C) ===\n");
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
        B.reloadTime = 30.0;
    } else if (B.type == 'R' || B.type == 'r') {
        sprintf(B.name, "Richelieu");
        B.reloadTime = 32.0;
    } else if (B.type == 'S' || B.type == 's') {
        sprintf(B.name, "Sovetsky Soyuz-class");
        B.reloadTime = 35.0;
    } else {
        B.type = 'U';
        sprintf(B.name, "USS Iowa (BB-61)");
        B.reloadTime = 20.0;
    }

    B.maxVel = 400 + rand() % 400;
    B.maxRange = getRange(B.maxVel);
    B.cumulativeImpact = 0.0;
    B.nextFireTime = 0.0;

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
        E[i].nextFireTime = 0.0;

        E[i].minAngle = 10 + rand() % 20;
        E[i].minVel = 50 + rand() % 50;

        if (E[i].type == 0) {
            sprintf(E[i].typeName, "1936A-class Destroyer");
            E[i].maxAngle = E[i].minAngle + 20;
            E[i].maxVel = 1.2 * B.maxVel;
            E[i].impactPower = 0.08;
            E[i].reloadTime = 12.0;
        } else if (E[i].type == 1) {
            sprintf(E[i].typeName, "Gabbiano-class Corvette");
            E[i].maxAngle = E[i].minAngle + 30;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.06;
            E[i].reloadTime = 15.0;
        } else if (E[i].type == 2) {
            sprintf(E[i].typeName, "Matsu-class Destroyer");
            E[i].maxAngle = E[i].minAngle + 25;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.07;
            E[i].reloadTime = 14.0;
        } else if (E[i].type == 3) {
            sprintf(E[i].typeName, "F-class Escort Ship");
            E[i].maxAngle = E[i].minAngle + 50;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.05;
            E[i].reloadTime = 18.0;
        } else {
            sprintf(E[i].typeName, "Japanese Kaibokan");
            E[i].maxAngle = E[i].minAngle + 70;
            E[i].maxVel = (rand() % (int)(B.maxVel * 100)) / 100.0;
            E[i].impactPower = 0.04;
            E[i].reloadTime = 20.0;
        }

        E[i].maxRange = getRange(E[i].maxVel);
    }

    // Save Initial Conditions File
    FILE *fInit = fopen("part2c_initial_conditions.txt", "w");
    if (fInit != NULL) {
        fprintf(fInit, "INITIAL BATTLEFIELD CONDITIONS (PART 2-C)\n");
        fprintf(fInit, "Battleship: %s | Reload Time (T_B): %.1fs\n\n", B.name, B.reloadTime);
        fprintf(fInit, "ID\tType\t\t\t\tX\tY\tSpeed\tImpactPower\tReloadTime\tMaxRange\n");
        for (int i = 0; i < N; i++) {
            fprintf(fInit, "%d\t%-25s\t%.1f\t%.1f\t%.1f\t%.2f\t\t%.1fs\t\t%.2f\n",
                    E[i].id, E[i].typeName, E[i].x, E[i].y, E[i].maxVel, 
                    E[i].impactPower, E[i].reloadTime, E[i].maxRange);
        }
        fclose(fInit);
    }

    // ==========================================
    // SIMULATION WITH MOVING ESCORT SHIPS
    // ==========================================
    printf("\n--- RUNNING PART 2-C SIMULATION (Dynamic Movement) ---\n");
    FILE *fSim = fopen("part2c_simulation_results.txt", "w");

    int bSunk = 0;
    int sunkStep = -1;
    double currentTime = 0.0;
    double timeStep = 1.0; // 1-second simulation step

    for (int k = 0; k < K; k++) {
        B.x = path[k].x;
        B.y = path[k].y;

        if (fSim != NULL) {
            fprintf(fSim, "\n--- Step %d at Position (%.1f, %.1f) | Time: %.1fs ---\n", k + 1, B.x, B.y, currentTime);
        }

        double stepEndTime = currentTime + 60.0; // Spend 60s moving toward/engaging at this waypoint

        while (currentTime < stepEndTime) {
            // 1. Dynamic Position Updates for Active Escort Ships
            for (int i = 0; i < N; i++) {
                updateEscortPosition(&E[i], B, timeStep);
            }

            // 2. Active Escort Ships Attack Battleship
            for (int i = 0; i < N; i++) {
                if (!E[i].isDestroyed && currentTime >= E[i].nextFireTime) {
                    double dist = getDistance(B.x, B.y, E[i].x, E[i].y);
                    if (dist <= E[i].maxRange) {
                        B.cumulativeImpact += E[i].impactPower;
                        E[i].nextFireTime = currentTime + E[i].reloadTime;

                        if (fSim != NULL) {
                            fprintf(fSim, "[T=%.1fs] E-Ship ID %d fired from (%.1f, %.1f)! Impact: +%.2f (Total: %.2f)\n",
                                    currentTime, E[i].id, E[i].x, E[i].y, E[i].impactPower, B.cumulativeImpact);
                        }

                        if (B.cumulativeImpact >= 1.0) {
                            bSunk = 1;
                            sunkStep = k + 1;
                            if (fSim != NULL) {
                                fprintf(fSim, "[T=%.1fs] Battleship %s SUNK at step %d!\n", currentTime, B.name, k + 1);
                            }
                            break;
                        }
                    }
                }
            }

            if (bSunk) break;

            // 3. Battleship Fires at Highest Priority Target in Range
            if (currentTime >= B.nextFireTime) {
                int targetIdx = selectTargetStrategy(B, E, N);
                if (targetIdx != -1) {
                    E[targetIdx].isDestroyed = 1;
                    B.nextFireTime = currentTime + B.reloadTime;

                    if (fSim != NULL) {
                        fprintf(fSim, "[T=%.1fs] Battleship destroyed moving E-Ship ID %d (%s) -> Next B shot at T=%.1fs\n",
                                currentTime, E[targetIdx].id, E[targetIdx].typeName, B.nextFireTime);
                    }
                }
            }

            currentTime += timeStep;
        }

        if (bSunk) break;
    }

    if (fSim != NULL) {
        fprintf(fSim, "\nFINAL SUMMARY (PART 2-C):\n");
        fprintf(fSim, "Battleship Status: %s\n", bSunk ? "SUNK" : "SURVIVED");
        fprintf(fSim, "Cumulative Impact Taken: %.2f (%.1f%%)\n", B.cumulativeImpact, B.cumulativeImpact * 100.0);
        fprintf(fSim, "Total Simulation Time Elapsed: %.1f seconds\n", currentTime);
        fclose(fSim);
    }

    if (bSunk) {
        printf("Simulation Result: Battleship SUNK at path point %d!\n", sunkStep);
    } else {
        printf("Simulation Result: Battleship SURVIVED! Cumulative Impact: %.2f (%.1f%%)\n",
               B.cumulativeImpact, B.cumulativeImpact * 100.0);
        printf("Total Simulation Time Elapsed: %.1f seconds\n", currentTime);
    }

    printf("\nAll results saved to 'part2c_initial_conditions.txt' and 'part2c_simulation_results.txt'.\n");

    return 0;
}
