#pragma once
enum ComponentType {
    COMP_TRANSFORM,
    COMP_SPRITE,
    COMP_ANIMATOR,
};

struct Component {
    ComponentType type;
};