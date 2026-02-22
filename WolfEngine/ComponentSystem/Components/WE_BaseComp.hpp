#pragma once
enum ComponentType {
    COMP_TRANSFORM,
    COMP_SPRITE,
};

struct Component {
    ComponentType type;
};