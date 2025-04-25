#include "Kammy.h"

NpcSettings N(NpcSettings_Kammy) = {
    .height = 40,
    .radius = 30,
    .onHit = &EnemyNpcHit,
    .onDefeat = &EnemyNpcDefeat,
    .level = ACTOR_LEVEL_MAGIKOOPA,
};
