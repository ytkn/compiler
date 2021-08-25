#include "9cc.h"

int calc_size(TypeKind ty) {
    switch (ty) {
        case TP_INT:
            return 4;
        case TP_PTR:
            return 8;
    }
}