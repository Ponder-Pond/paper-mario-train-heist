#include "grs_01.h"
#include "foliage.hpp"

namespace grs_01 {

DEFINE_TREE(Tree1, MODEL_TreeLeaves1, MODEL_TreeTrunk1, GEN_TREE1_VEC, 0.0f)
DEFINE_TREE(Tree2, MODEL_TreeLeaves2, MODEL_TreeTrunk2, GEN_TREE2_VEC, 0.0f)
DEFINE_TREE(Tree3, MODEL_TreeLeaves3, MODEL_TreeTrunk3, GEN_TREE3_VEC, 0.0f)

DEFINE_BUSH(Bush1, MODEL_Bush1, GEN_BUSH1_VEC)
DEFINE_BUSH(Bush2, MODEL_Bush2, GEN_BUSH2_VEC)
DEFINE_BUSH(Bush3, MODEL_Bush3, GEN_BUSH3_VEC)
DEFINE_BUSH(Bush4, MODEL_Bush4, GEN_BUSH4_VEC)
DEFINE_BUSH(Bush5, MODEL_Bush5, GEN_BUSH5_VEC)
DEFINE_BUSH(Bush6, MODEL_Bush6, GEN_BUSH6_VEC)
DEFINE_BUSH(Bush7, MODEL_Bush7, GEN_BUSH7_VEC)
DEFINE_BUSH(Bush8, MODEL_Bush8, GEN_BUSH8_VEC)

EvtScript EVS_SetFoliage = {
    BIND_TREE(Tree1, COLLIDER_Tree1)
    BIND_TREE(Tree2, COLLIDER_Tree2)
    BIND_TREE(Tree3, COLLIDER_Tree3)
    BIND_BUSH(Bush1, COLLIDER_Bush1)
    BIND_BUSH(Bush2, COLLIDER_Bush2)
    BIND_BUSH(Bush3, COLLIDER_Bush3)
    BIND_BUSH(Bush4, COLLIDER_Bush4)
    BIND_BUSH(Bush5, COLLIDER_Bush5)
    BIND_BUSH(Bush6, COLLIDER_Bush6)
    BIND_BUSH(Bush7, COLLIDER_Bush7)
    BIND_BUSH(Bush8, COLLIDER_Bush8)
    Return
    End
};

}; // namespace grs_01
