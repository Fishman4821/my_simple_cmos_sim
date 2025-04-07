#include <iostream>
#include <vector>
#include <string.h>

#define SDL_MAIN_HANDLED

#include "mgui.cpp"

using namespace std;

void draw_grid(State* state, int view_x, int view_y, float zoom, int grid_spacing) {
    const Color background = Color(27, 28, 32);
    const Color dot = Color(22, 22, 22);
    const Color line = Color(50, 50, 50, 75);

    int spacing_zoom = grid_spacing * zoom;
    int row_initial = view_y % spacing_zoom;

    int w = state->r.w;
    int h = state->r.h;

    int column = view_x % spacing_zoom;
    int row = row_initial;

    state->r.rect(0, 0, w, h, background);

    while (column - spacing_zoom <= w) {
        while (row - spacing_zoom <= h) {
            state->r.line(column, row, column - spacing_zoom, row, line);
            state->r.line(column, row, column, row - spacing_zoom, line);
            row += spacing_zoom;
        }
        row = row_initial;

        column += spacing_zoom;
    }
    column = view_x % spacing_zoom;
    while (column - spacing_zoom <= w) {
        while (row - spacing_zoom <= h) {
            state->r.rect(column - 1, row - 1, column + 2, row + 2, dot);
            row += spacing_zoom;
        }
        row = row_initial;

        column += spacing_zoom;
    }

}

struct Wire {
    int x1, y1;
    int x2, y2;
    bool selected;
    Wire* prev;
    Wire* next;
};

void delete_wire(Wire** wires, Wire* wire) {
    if (wire->prev == nullptr) {
        *wires = wire->next;
    } else {
        wire->prev->next = wire->next;
    }
    if (wire->next != nullptr) {
        wire->next->prev = wire->prev;
    }
    free(wire);
}

void delist_wire(Wire** wires, Wire* wire) {
    if (wire->prev == nullptr) {
        *wires = wire->next;
    } else {
        wire->prev->next = wire->next;
    }
    if (wire->next != nullptr) {
        wire->next->prev = wire->prev;
    }
    wire->prev = 0;
    wire->next = 0;
}

void append_wire(Wire** wires, Wire wire) {
    Wire* next = (Wire*)malloc(sizeof(Wire));
    *next = wire;
    if (*wires == nullptr) {
        *wires = next;
        return;
    }
    Wire* end;
    Wire* current = *wires;
    while (current != nullptr) {
        end = current;
        current = current->next;
    }
    end->next = next;
    next->prev = end;
}

void free_wires(Wire* wires) {
    if (wires != nullptr) {
        Wire* current = wires;
        Wire* temp;
        while (current != nullptr) {
            temp = current->next;
            free(current);
            current = temp;
        }
    }
}

struct Node {
    char state;
    Wire* wires;
    Node* prev;
    Node* next;
};

void delete_node(Node** nodes, Node* node, bool wires) {
    if (node->prev == nullptr) {
        *nodes = node->next;
    } else {
        node->prev->next = node->next;
    }
    if (node->next != nullptr) {
        node->next->prev = node->prev;
    }
    if (wires) { 
        free_wires(node->wires);
    }
    free(node);
}

void append_node(Node** nodes, Node node) {
    Node* next = (Node*)malloc(sizeof(Node));
    *next = node;
    if (*nodes == nullptr) {
        *nodes = next;
        return;
    }
    Node* end;
    Node* current = *nodes;
    while (current != nullptr) {
        end = current;
        current = current->next;
    }
    end->next = next;
    next->prev = end;
}

void free_nodes(Node* nodes) {
    Node* current = nodes;
    Node* temp;
    while (current != nullptr) {
        temp = current->next;
        free_wires(current->wires);
        free(current);
        current = temp;
    }
}

struct Object {
    char type;
    char state;
    int x, y;
    char rotation;
    Node* a; 
    Node* b; 
    Node* c;
    bool selected;
    Object* prev;
    Object* next;
};

void delete_object(Object** objects, Object* object) {
    if (object->prev == nullptr) {
        *objects = object->next;
    } else {
        object->prev->next = object->next;
    }
    if (object->next != nullptr) {
        object->next->prev = object->prev;
    }
    free(object);
}

void append_object(Object** objects, Object object) {
    Object* next = (Object*)malloc(sizeof(Object));
    *next = object;
    if (*objects == nullptr) {
        *objects = next;
        return;
    }
    Object* end;
    Object* previous;
    Object* current = *objects;
    while (current != nullptr) {
        end = current;
        current = current->next;
    }
    end->next = next;
    next->prev = end;
}

void free_objects(Object* objects) {
    Object* current = objects;
    Object* temp;
    while (current != nullptr) {
        temp = current->next;
        free(current);
        current = temp;
    }
}

const Color wire_disconnected = Color(66, 60, 68);
const Color wire_on = Color(50, 150, 0);
const Color wire_off = Color(0, 65, 0);
const Color wire_conflicted = Color(140, 140, 0);

const Color selection = Color(255, 255, 255, 35);

void draw_nmos_symbol(State* state, int x, int y, char rot, int view_x, int view_y, int zoom, bool selected, Color a, Color b, Color c) {
    if (rot == 0 || rot == 2) {
        int top_y = y * zoom;
        int bottom_y = top_y + zoom;
        int bottom_1_3_y = bottom_y - zoom / 3.0;
        int left_x = x * zoom;
        int left_1_3_x = left_x + zoom / 3.0;
        int right_x = x * zoom + (zoom * 2);
        int right_1_3_x = right_x - zoom / 3.0;
        int left_1_2_x = left_x + zoom / 2.0;
        int right_1_2_x = right_x - zoom / 2.0;
        int middle_y = bottom_y - zoom / 2.0;
        int middle_x = left_x + zoom;
        int top_1_3_y = top_y + zoom / 3.0;
        if (rot == 0) {
            state->r.line(left_x + view_x, bottom_y + view_y, left_1_3_x + view_x, bottom_y + view_y, a);
            state->r.line(left_1_3_x + view_x, bottom_y + view_y, left_1_3_x + view_x, bottom_1_3_y + view_y, a);
            state->r.line(left_1_3_x + view_x, bottom_1_3_y + view_y, middle_x + view_x, bottom_1_3_y + view_y, a);
            state->r.line(middle_x + view_x, bottom_1_3_y + view_y, right_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(right_1_3_x + view_x, bottom_y + view_y, right_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(right_x + view_x, bottom_y + view_y, right_1_3_x + view_x, bottom_y + view_y, c);

            state->r.line(left_1_2_x + view_x, middle_y + view_y, right_1_2_x + view_x, middle_y + view_y, b);
            state->r.line(middle_x + view_x, middle_y + view_y, middle_x + view_x, top_y + view_y, b);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        } else {
            state->r.line(left_x + view_x, top_y + view_y, left_1_3_x + view_x, top_y + view_y, a);
            state->r.line(left_1_3_x + view_x, top_y + view_y, left_1_3_x + view_x, top_1_3_y + view_y, a);
            state->r.line(left_1_3_x + view_x, top_1_3_y + view_y, middle_x + view_x, top_1_3_y + view_y, a);
            state->r.line(middle_x + view_x, top_1_3_y + view_y, right_1_3_x + view_x, top_1_3_y + view_y, c);
            state->r.line(right_1_3_x + view_x, top_y + view_y, right_1_3_x + view_x, top_1_3_y + view_y, c);
            state->r.line(right_x + view_x, top_y + view_y, right_1_3_x + view_x, top_y + view_y, c);

            state->r.line(left_1_2_x + view_x, middle_y + view_y, right_1_2_x + view_x, middle_y + view_y, b);
            state->r.line(middle_x + view_x, middle_y + view_y, middle_x + view_x, bottom_y + view_y, b);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        }
    } else {
        int left_x = x * zoom;
        int right_x = left_x + zoom;
        int left_1_3_x = left_x + zoom / 3.0;
        int top_y = y * zoom;
        int top_1_3_y = top_y + zoom / 3.0;
        int bottom_y = y * zoom + (zoom * 2);
        int bottom_1_3_y = bottom_y - zoom / 3.0;
        int top_1_2_y = top_y + zoom / 2.0;
        int bottom_1_2_y = bottom_y - zoom / 2.0;
        int middle_x = right_x - zoom / 2.0;
        int middle_y = top_y + zoom;
        int right_1_3_x = right_x - zoom / 3.0;
        if (rot == 1) {
            state->r.line(left_x + view_x, top_y + view_y, left_x + view_x, top_1_3_y + view_y, a);
            state->r.line(left_x + view_x, top_1_3_y + view_y, left_1_3_x + view_x, top_1_3_y + view_y, a);
            state->r.line(left_1_3_x + view_x, top_1_3_y + view_y, left_1_3_x + view_x, middle_y + view_y, a);
            state->r.line(left_1_3_x + view_x, middle_y + view_y, left_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(left_x + view_x, bottom_1_3_y + view_y, left_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(left_x + view_x, bottom_y + view_y, left_x + view_x, bottom_1_3_y + view_y, c);

            state->r.line(middle_x + view_x, top_1_2_y + view_y, middle_x + view_x, bottom_1_2_y + view_y, b);
            state->r.line(middle_x + view_x, middle_y + view_y, right_x + view_x, middle_y + view_y, b);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        } else {
            state->r.line(right_x + view_x, top_y + view_y, right_x + view_x, top_1_3_y + view_y, a);
            state->r.line(right_x + view_x, top_1_3_y + view_y, right_1_3_x + view_x, top_1_3_y + view_y, a);
            state->r.line(right_1_3_x + view_x, top_1_3_y + view_y, right_1_3_x + view_x, middle_y + view_y, a);
            state->r.line(right_1_3_x + view_x, middle_y + view_y, right_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(right_x + view_x, bottom_1_3_y + view_y, right_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(right_x + view_x, bottom_y + view_y, right_x + view_x, bottom_1_3_y + view_y, c);

            state->r.line(middle_x + view_x, top_1_2_y + view_y, middle_x + view_x, bottom_1_2_y + view_y, b);
            state->r.line(middle_x + view_x, middle_y + view_y, left_x + view_x, middle_y + view_y, b);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        }
    }
}

void draw_nmos(State* state, Object* object, Node* nodes, int view_x, int view_y, int spacing_zoom) {
    const Color* a_color;
    const Color* b_color;
    const Color* c_color;
    if (object->a != nullptr) {
        if (object->a->state == 0b00) {
            a_color = &wire_disconnected;
        } else if (object->a->state == 0b01) {
            a_color = &wire_off;
        } else if (object->a->state == 0b10) {
            a_color = &wire_on;
        } else if (object->a->state == 0b11) {
            a_color = &wire_conflicted;
        }
    } else {
        if (object->state == 0b00) {
            a_color = &wire_disconnected;
        } else if (object->state == 0b01) {
            a_color = &wire_off;
        } else if (object->state == 0b10) {
            a_color = &wire_on;
        } else if (object->state == 0b11) {
            a_color = &wire_conflicted;
        }
    }

    if (object->b == nullptr || object->b->state == 0b00) {
        b_color = &wire_disconnected;
    } else if (object->b->state == 0b01) {
        b_color = &wire_off;
    } else if (object->b->state == 0b10) {
        b_color = &wire_on;
    } else if (object->b->state == 0b11) {
        b_color = &wire_conflicted;
    }

    if (object->c != nullptr) {
        if (object->c->state == 0b00) {
            c_color = &wire_disconnected;
        } else if (object->c->state == 0b01) {
            c_color = &wire_off;
        } else if (object->c->state == 0b10) {
            c_color = &wire_on;
        } else if (object->c->state == 0b11) {
            c_color = &wire_conflicted;
        }
    } else {
        if (object->state == 0b00) {
            c_color = &wire_disconnected;
        } else if (object->state == 0b01) {
            c_color = &wire_off;
        } else if (object->state == 0b10) {
            c_color = &wire_on;
        } else if (object->state == 0b11) {
            c_color = &wire_conflicted;
        }
    }

    draw_nmos_symbol(state, object->x, object->y, object->rotation, view_x, view_y, spacing_zoom, object->selected, *a_color, *b_color , *c_color);
}

void draw_pmos_symbol(State* state, int x, int y, char rot, int view_x, int view_y, int zoom, bool selected, Color a, Color b, Color c) {
    if (rot == 0 || rot == 2) {
        int top_y = y * zoom;
        int bottom_y = top_y + zoom;
        int bottom_1_3_y = bottom_y - zoom / 3.0;
        int left_x = x * zoom;
        int left_1_3_x = left_x + zoom / 3.0;
        int right_x = x * zoom + (zoom * 2);
        int right_1_3_x = right_x - zoom / 3.0;
        int left_1_2_x = left_x + zoom / 2.0;
        int right_1_2_x = right_x - zoom / 2.0;
        int middle_y = bottom_y - zoom / 2.0;
        int middle_x = left_x + zoom;
        int top_1_3_y = top_y + zoom / 3.0;
        int zoom_1_12 = zoom / 12.0f;
        if (rot == 0) {
            state->r.line(left_x + view_x, bottom_y + view_y, left_1_3_x + view_x, bottom_y + view_y, a);
            state->r.line(left_1_3_x + view_x, bottom_y + view_y, left_1_3_x + view_x, bottom_1_3_y + view_y, a);
            state->r.line(left_1_3_x + view_x, bottom_1_3_y + view_y, middle_x + view_x, bottom_1_3_y + view_y, a);
            state->r.line(middle_x + view_x, bottom_1_3_y + view_y, right_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(right_1_3_x + view_x, bottom_y + view_y, right_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(right_x + view_x, bottom_y + view_y, right_1_3_x + view_x, bottom_y + view_y, c);

            state->r.line(left_1_2_x + view_x, middle_y + view_y, right_1_2_x + view_x, middle_y + view_y, b);
            state->r.line(middle_x + view_x, top_1_3_y + view_y, middle_x + view_x, top_y + view_y, b);

            state->r.line(middle_x + view_x, top_1_3_y + view_y, middle_x - zoom_1_12 + view_x, top_1_3_y + zoom_1_12 + view_y, b);
            state->r.line(middle_x - zoom_1_12 + view_x, top_1_3_y + zoom_1_12 + view_y, middle_x + view_x, middle_y + view_y, b);
            state->r.line(middle_x + view_x, middle_y + view_y, middle_x + zoom_1_12 + view_x, top_1_3_y + zoom_1_12 + view_y, b);
            state->r.line(middle_x + zoom_1_12 + view_x, top_1_3_y + zoom_1_12 + view_y, middle_x + view_x, top_1_3_y + view_y, b);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        } else {
            state->r.line(left_x + view_x, top_y + view_y, left_1_3_x + view_x, top_y + view_y, a);
            state->r.line(left_1_3_x + view_x, top_y + view_y, left_1_3_x + view_x, top_1_3_y + view_y, a);
            state->r.line(left_1_3_x + view_x, top_1_3_y + view_y, middle_x + view_x, top_1_3_y + view_y, a);
            state->r.line(middle_x + view_x, top_1_3_y + view_y, right_1_3_x + view_x, top_1_3_y + view_y, c);
            state->r.line(right_1_3_x + view_x, top_y + view_y, right_1_3_x + view_x, top_1_3_y + view_y, c);
            state->r.line(right_x + view_x, top_y + view_y, right_1_3_x + view_x, top_y + view_y, c);

            state->r.line(left_1_2_x + view_x, middle_y + view_y, right_1_2_x + view_x, middle_y + view_y, b);
            state->r.line(middle_x + view_x, bottom_1_3_y + view_y, middle_x + view_x, bottom_y + view_y, b);

            state->r.line(middle_x + view_x, bottom_1_3_y + view_y, middle_x - zoom_1_12 + view_x, bottom_1_3_y - zoom_1_12 + view_y, b);
            state->r.line(middle_x - zoom_1_12 + view_x, bottom_1_3_y - zoom_1_12 + view_y, middle_x + view_x, middle_y + view_y, b);
            state->r.line(middle_x + view_x, middle_y + view_y, middle_x + zoom_1_12 + view_x, bottom_1_3_y - zoom_1_12 + view_y, b);
            state->r.line(middle_x + zoom_1_12 + view_x, bottom_1_3_y - zoom_1_12 + view_y, middle_x + view_x, bottom_1_3_y + view_y, b);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        }
    } else {
        int left_x = x * zoom;
        int right_x = left_x + zoom;
        int left_1_3_x = left_x + zoom / 3.0;
        int top_y = y * zoom;
        int top_1_3_y = top_y + zoom / 3.0;
        int bottom_y = y * zoom + (zoom * 2);
        int bottom_1_3_y = bottom_y - zoom / 3.0;
        int top_1_2_y = top_y + zoom / 2.0;
        int bottom_1_2_y = bottom_y - zoom / 2.0;
        int middle_x = right_x - zoom / 2.0;
        int middle_y = top_y + zoom;
        int right_1_3_x = right_x - zoom / 3.0;
        int zoom_1_12 = zoom / 12.0;
        if (rot == 1) {
            state->r.line(left_x + view_x, top_y + view_y, left_x + view_x, top_1_3_y + view_y, a);
            state->r.line(left_x + view_x, top_1_3_y + view_y, left_1_3_x + view_x, top_1_3_y + view_y, a);
            state->r.line(left_1_3_x + view_x, top_1_3_y + view_y, left_1_3_x + view_x, middle_y + view_y, a);
            state->r.line(left_1_3_x + view_x, middle_y + view_y, left_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(left_x + view_x, bottom_1_3_y + view_y, left_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(left_x + view_x, bottom_y + view_y, left_x + view_x, bottom_1_3_y + view_y, c);

            state->r.line(middle_x + view_x, top_1_2_y + view_y, middle_x + view_x, bottom_1_2_y + view_y, b);
            state->r.line(right_1_3_x + view_x, middle_y + view_y, right_x + view_x, middle_y + view_y, b);

            state->r.line(right_1_3_x + view_x, middle_y + view_y, right_1_3_x - zoom_1_12 + view_x, middle_y - zoom_1_12 + view_y, b);
            state->r.line(right_1_3_x - zoom_1_12 + view_x, middle_y - zoom_1_12 + view_y, middle_x + view_x, middle_y + view_y, b);
            state->r.line(middle_x + view_x, middle_y + view_y, right_1_3_x - zoom_1_12 + view_x, middle_y + zoom_1_12 + view_y, b);
            state->r.line(right_1_3_x - zoom_1_12 + view_x, middle_y + zoom_1_12 + view_y, right_1_3_x + view_x, middle_y + view_y, b);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        } else {
            state->r.line(right_x + view_x, top_y + view_y, right_x + view_x, top_1_3_y + view_y, a);
            state->r.line(right_x + view_x, top_1_3_y + view_y, right_1_3_x + view_x, top_1_3_y + view_y, a);
            state->r.line(right_1_3_x + view_x, top_1_3_y + view_y, right_1_3_x + view_x, middle_y + view_y, a);
            state->r.line(right_1_3_x + view_x, middle_y + view_y, right_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(right_x + view_x, bottom_1_3_y + view_y, right_1_3_x + view_x, bottom_1_3_y + view_y, c);
            state->r.line(right_x + view_x, bottom_y + view_y, right_x + view_x, bottom_1_3_y + view_y, c);

            state->r.line(middle_x + view_x, top_1_2_y + view_y, middle_x + view_x, bottom_1_2_y + view_y, b);
            state->r.line(left_1_3_x + view_x, middle_y + view_y, left_x + view_x, middle_y + view_y, b);

            state->r.line(left_1_3_x + view_x, middle_y + view_y, left_1_3_x + zoom_1_12 + view_x, middle_y - zoom_1_12 + view_y, b);
            state->r.line(left_1_3_x + zoom_1_12 + view_x, middle_y - zoom_1_12 + view_y, middle_x + view_x, middle_y + view_y, b);
            state->r.line(middle_x + view_x, middle_y + view_y, left_1_3_x + zoom_1_12 + view_x, middle_y + zoom_1_12 + view_y, b);
            state->r.line(left_1_3_x + zoom_1_12 + view_x, middle_y + zoom_1_12 + view_y, left_1_3_x + view_x, middle_y + view_y, b);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        }
    }
}

void draw_pmos(State* state, Object* object, Node* nodes, int view_x, int view_y, int spacing_zoom) {
    const Color* a_color;
    const Color* b_color;
    const Color* c_color;
    if (object->a != nullptr) {
        if (object->a->state == 0b00) {
            a_color = &wire_disconnected;
        } else if (object->a->state == 0b01) {
            a_color = &wire_off;
        } else if (object->a->state == 0b10) {
            a_color = &wire_on;
        } else if (object->a->state == 0b11) {
            a_color = &wire_conflicted;
        }
    } else {
        if (object->state == 0b00) {
            a_color = &wire_disconnected;
        } else if (object->state == 0b01) {
            a_color = &wire_off;
        } else if (object->state == 0b10) {
            a_color = &wire_on;
        } else if (object->state == 0b11) {
            a_color = &wire_conflicted;
        }
    }

    if (object->b == nullptr || object->b->state == 0b00) {
        b_color = &wire_disconnected;
    } else if (object->b->state == 0b01) {
        b_color = &wire_off;
    } else if (object->b->state == 0b10) {
        b_color = &wire_on;
    } else if (object->b->state == 0b11) {
        b_color = &wire_conflicted;
    }

    if (object->c != nullptr) {
        if (object->c->state == 0b00) {
            c_color = &wire_disconnected;
        } else if (object->c->state == 0b01) {
            c_color = &wire_off;
        } else if (object->c->state == 0b10) {
            c_color = &wire_on;
        } else if (object->c->state == 0b11) {
            c_color = &wire_conflicted;
        }
    } else {
        if (object->state == 0b00) {
            c_color = &wire_disconnected;
        } else if (object->state == 0b01) {
            c_color = &wire_off;
        } else if (object->state == 0b10) {
            c_color = &wire_on;
        } else if (object->state == 0b11) {
            c_color = &wire_conflicted;
        }
    }

    draw_pmos_symbol(state, object->x, object->y, object->rotation, view_x, view_y, spacing_zoom, object->selected, *a_color, *b_color, *c_color);
}

void draw_vplus_symbol(State* state, int x, int y, char rot, int view_x, int view_y, int zoom, bool selected, Color a) {
    if (rot == 0 || rot == 2) {
        int middle_x = x * zoom;
        int left_x = middle_x - zoom * 0.176777; // 0.176777 = sqrt(2) / 8
        int right_x = middle_x + zoom * 0.176777;
        if (rot == 0) {
            int bottom_y = y * zoom;
            int middle_y = bottom_y - zoom / 2.0;
            int top_y = bottom_y - zoom * 0.707107; // 0.707107 = sqrt(2) / 2

            state->r.line(middle_x + view_x, bottom_y + view_y, middle_x + view_x, middle_y + view_y, a);
            state->r.line(left_x + view_x, middle_y + view_y, right_x + view_x, middle_y + view_y, a);
            state->r.line(left_x + view_x, middle_y + view_y, middle_x + view_x, top_y + view_y, a);
            state->r.line(right_x + view_x, middle_y + view_y, middle_x + view_x, top_y + view_y, a);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        } else {
            int top_y = y * zoom;
            int middle_y = top_y + zoom / 2.0;
            int bottom_y = top_y + zoom * 0.707107; // 0.707107 = sqrt(2) / 2

            state->r.line(middle_x + view_x, top_y + view_y, middle_x + view_x, middle_y + view_y, a);
            state->r.line(left_x + view_x, middle_y + view_y, right_x + view_x, middle_y + view_y, a);
            state->r.line(left_x + view_x, middle_y + view_y, middle_x + view_x, bottom_y + view_y, a);
            state->r.line(right_x + view_x, middle_y + view_y, middle_x + view_x, bottom_y + view_y, a);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y - 3, right_x + view_x + 3, bottom_y + view_y + 3, selection); }
        }
    } else {
        int middle_y = y * zoom;
        int bottom_y = middle_y - zoom * 0.176777; // 0.176777 = sqrt(2) / 8
        int top_y = middle_y + zoom * 0.176777;
        if (rot == 1) {
            int left_x = x * zoom;
            int middle_x = left_x + zoom / 2.0;
            int right_x = left_x + zoom * 0.707107; // 0.707107 = sqrt(2) / 2

            state->r.line(left_x + view_x, middle_y + view_y, middle_x + view_x, middle_y + view_y, a);
            state->r.line(middle_x + view_x, top_y + view_y, middle_x + view_x, bottom_y + view_y, a);
            state->r.line(middle_x + view_x, top_y + view_y, right_x + view_x, middle_y + view_y, a);
            state->r.line(middle_x + view_x, bottom_y + view_y, right_x + view_x, middle_y + view_y, a);
            if (selected) { state->r.rect(left_x + view_x - 3, top_y + view_y + 3, right_x + view_x + 3, bottom_y + view_y - 3, selection); }
        } else {
            int left_x = x * zoom;
            int middle_x = left_x - zoom / 2.0;
            int right_x = left_x - zoom * 0.707107; // 0.707107 = sqrt(2) / 2

            state->r.line(left_x + view_x, middle_y + view_y, middle_x + view_x, middle_y + view_y, a);
            state->r.line(middle_x + view_x, top_y + view_y, middle_x + view_x, bottom_y + view_y, a);
            state->r.line(middle_x + view_x, top_y + view_y, right_x + view_x, middle_y + view_y, a);
            state->r.line(middle_x + view_x, bottom_y + view_y, right_x + view_x, middle_y + view_y, a);
            if (selected) { state->r.rect(left_x + view_x + 3, top_y + view_y + 3, right_x + view_x - 3, bottom_y + view_y - 3, selection); }
        }
    }
}

void draw_vplus(State* state, Object* object, Node* nodes, int view_x, int view_y, int spacing_zoom) {
    draw_vplus_symbol(state, object->x, object->y, object->rotation, view_x, view_y, spacing_zoom, object->selected, wire_on);
}

void draw_vminus_symbol(State* state, int x, int y, char rot, int view_x, int view_y, int zoom, bool selected, Color a) {
    if (rot == 0 || rot == 2) {
        int middle_x = x * zoom;
        int t_1_x = zoom * 0.176777; // 0.176777 = sqrt(2) / 8
        int t_2_x = zoom * 0.117851; // 0.117851 = sqrt(2) / 12
        int t_3_x = zoom * 0.058926; // 0.058926 = sqrt(2) / 24
        int start_y = y * zoom;
        int t_1_y = zoom / 2.0;
        int t_2_y = t_1_y + zoom * 0.078567; // 0.078567 = sqrt(2) / 18
        int t_3_y = t_1_y + zoom * 0.141421; // 0.141421 = sqrt(2) / 10

        if (rot == 0) {
            state->r.line(middle_x + view_x, start_y + view_y, middle_x + view_x, start_y - t_1_y + view_y, a);
            state->r.line(middle_x - t_1_x + view_x, start_y - t_1_y + view_y, middle_x + t_1_x + view_x, start_y - t_1_y + view_y, a);
            state->r.line(middle_x - t_2_x + view_x, start_y - t_2_y + view_y, middle_x + t_2_x + view_x, start_y - t_2_y + view_y, a);
            state->r.line(middle_x - t_3_x + view_x, start_y - t_3_y + view_y, middle_x + t_3_x + view_x, start_y - t_3_y + view_y, a);
            if (selected) { state->r.rect(middle_x - t_1_x + view_x - 3, start_y - t_3_y + view_y - 3, middle_x + t_1_x + view_x + 3, start_y + view_y + 3, selection); }
        } else {
            state->r.line(middle_x + view_x, start_y + view_y, middle_x + view_x, start_y + t_1_y + view_y, a);
            state->r.line(middle_x - t_1_x + view_x, start_y + t_1_y + view_y, middle_x + t_1_x + view_x, start_y + t_1_y + view_y, a);
            state->r.line(middle_x - t_2_x + view_x, start_y + t_2_y + view_y, middle_x + t_2_x + view_x, start_y + t_2_y + view_y, a);
            state->r.line(middle_x - t_3_x + view_x, start_y + t_3_y + view_y, middle_x + t_3_x + view_x, start_y + t_3_y + view_y, a);
            if (selected) { state->r.rect(middle_x - t_1_x + view_x - 3, start_y + view_y - 3, middle_x + t_1_x + view_x + 3, start_y + t_3_y + view_y + 3, selection); }
        }
    } else {
        int middle_y = y * zoom;
        int t_1_y = zoom * 0.176777; // 0.176777 = sqrt(2) / 8
        int t_2_y = zoom * 0.117851; // 0.117851 = sqrt(2) / 12
        int t_3_y = zoom * 0.058926; // 0.058926 = sqrt(2) / 24
        int start_x = x * zoom;
        int t_1_x = zoom / 2.0;
        int t_2_x = t_1_x + zoom * 0.078567; // 0.078567 = sqrt(2) / 18
        int t_3_x = t_1_x + zoom * 0.141421; // 0.141421 = sqrt(2) / 10

        if (rot == 1) {
            state->r.line(start_x + view_x, middle_y + view_y, start_x + t_1_x + view_x, middle_y + view_y, a);
            state->r.line(start_x + t_1_x + view_x, middle_y - t_1_y + view_y, start_x + t_1_x + view_x, middle_y + t_1_y + view_y, a);
            state->r.line(start_x + t_2_x + view_x, middle_y - t_2_y + view_y, start_x + t_2_x + view_x, middle_y + t_2_y + view_y, a);
            state->r.line(start_x + t_3_x + view_x, middle_y - t_3_y + view_y, start_x + t_3_x + view_x, middle_y + t_3_y + view_y, a);
            if (selected) { state->r.rect(start_x + view_x - 3, middle_y - t_1_y + view_y - 3, start_x + t_3_x + view_x + 3, middle_y + t_1_y + view_y + 3, selection); }
        } else {
            state->r.line(start_x + view_x, middle_y + view_y, start_x - t_1_x + view_x, middle_y + view_y, a);
            state->r.line(start_x - t_1_x + view_x, middle_y - t_1_y + view_y, start_x - t_1_x + view_x, middle_y + t_1_y + view_y, a);
            state->r.line(start_x - t_2_x + view_x, middle_y - t_2_y + view_y, start_x - t_2_x + view_x, middle_y + t_2_y + view_y, a);
            state->r.line(start_x - t_3_x + view_x, middle_y - t_3_y + view_y, start_x - t_3_x + view_x, middle_y + t_3_y + view_y, a);
            if (selected) { state->r.rect(start_x - t_3_x + view_x - 3, middle_y - t_1_y + view_y - 3, start_x + view_x + 3, middle_y + t_1_y + view_y + 3, selection); }
        }
    }
}

void draw_vminus(State* state, Object* object, Node* nodes, int view_x, int view_y, int spacing_zoom) {
    draw_vminus_symbol(state, object->x, object->y, object->rotation, view_x, view_y, spacing_zoom, object->selected, wire_off);
}

void draw_input_symbol(State* state, int x, int y, char rot, int view_x, int view_y, int zoom, bool selected, Color a, Color b) {
    int start_x = x * zoom + view_x;
    int start_y = y * zoom + view_y;
    int zoom_1_9 = zoom / 9.0;
    int zoom_1_6 = zoom / 6.0;
    int zoom_1_3 = zoom / 3.0;
    int zoom_2_3 = zoom * (2.0 / 3.0);

    if (rot == 0 || rot == 2) {
        if (rot == 0) {
            state->r.line(start_x, start_y, start_x + zoom_1_6, start_y - zoom_1_6, b);
            state->r.line(start_x + zoom_1_6, start_y - zoom_1_6, start_x + zoom_1_6, start_y - zoom_2_3, b);
            state->r.line(start_x, start_y, start_x - zoom_1_6, start_y - zoom_1_6, b);
            state->r.line(start_x - zoom_1_6, start_y - zoom_1_6, start_x - zoom_1_6, start_y - zoom_2_3, b);
            state->r.line(start_x + zoom_1_6, start_y - zoom_2_3, start_x - zoom_1_6, start_y - zoom_2_3, b);

            state->r.line(start_x, start_y, start_x, start_y - zoom_1_3, a);
            state->r.rect(start_x + zoom_1_9, start_y - zoom_1_3 - zoom_1_9 * 2, start_x - zoom_1_9, start_y - zoom_1_3, a);
            if (selected) { state->r.rect(start_x - zoom_1_6 - 3, start_y - zoom_2_3 - 3, start_x + zoom_1_6 + 3, start_y + 3, selection); }
        } else {
            state->r.line(start_x, start_y, start_x + zoom_1_6, start_y + zoom_1_6, b);
            state->r.line(start_x + zoom_1_6, start_y + zoom_1_6, start_x + zoom_1_6, start_y + zoom_2_3, b);
            state->r.line(start_x, start_y, start_x - zoom_1_6, start_y + zoom_1_6, b);
            state->r.line(start_x - zoom_1_6, start_y + zoom_1_6, start_x - zoom_1_6, start_y + zoom_2_3, b);
            state->r.line(start_x + zoom_1_6, start_y + zoom_2_3, start_x - zoom_1_6, start_y + zoom_2_3, b);

            state->r.line(start_x, start_y, start_x, start_y + zoom_1_3, a);
            state->r.rect(start_x + zoom_1_9, start_y + zoom_1_3 + zoom_1_9 * 2, start_x - zoom_1_9, start_y + zoom_1_3, a);
            if (selected) { state->r.rect(start_x - zoom_1_6 - 3, start_y - 3, start_x + zoom_1_6 + 3, start_y + zoom_2_3 + 3, selection); }
        }
    } else {
        if (rot == 1) {
            state->r.line(start_x, start_y, start_x + zoom_1_6, start_y + zoom_1_6, b);
            state->r.line(start_x + zoom_1_6, start_y + zoom_1_6, start_x + zoom_2_3, start_y + zoom_1_6, b);
            state->r.line(start_x, start_y, start_x + zoom_1_6, start_y - zoom_1_6, b);
            state->r.line(start_x + zoom_1_6, start_y - zoom_1_6, start_x + zoom_2_3, start_y - zoom_1_6, b);
            state->r.line(start_x + zoom_2_3, start_y + zoom_1_6, start_x + zoom_2_3, start_y - zoom_1_6, b);

            state->r.line(start_x, start_y, start_x + zoom_1_3, start_y, a);
            state->r.rect(start_x + zoom_1_3 + zoom_1_9 * 2, start_y + zoom_1_9, start_x + zoom_1_3, start_y - zoom_1_9, a);
            if (selected) { state->r.rect(start_x - 3, start_y - zoom_1_6 - 3, start_x + zoom_2_3 + 3, start_y + zoom_1_6 + 3, selection); }
        } else {
            state->r.line(start_x, start_y, start_x - zoom_1_6, start_y + zoom_1_6, b);
            state->r.line(start_x - zoom_1_6, start_y + zoom_1_6, start_x - zoom_2_3, start_y + zoom_1_6, b);
            state->r.line(start_x, start_y, start_x - zoom_1_6, start_y - zoom_1_6, b);
            state->r.line(start_x - zoom_1_6, start_y - zoom_1_6, start_x - zoom_2_3, start_y - zoom_1_6, b);
            state->r.line(start_x - zoom_2_3, start_y + zoom_1_6, start_x - zoom_2_3, start_y - zoom_1_6, b);

            state->r.line(start_x, start_y, start_x - zoom_1_3, start_y, a);
            state->r.rect(start_x - zoom_1_3 - zoom_1_9 * 2, start_y + zoom_1_9, start_x - zoom_1_3, start_y - zoom_1_9, a);
            if (selected) { state->r.rect(start_x - zoom_2_3 - 3, start_y - zoom_1_6 - 3, start_x + 3, start_y + zoom_1_6 + 3, selection); }
        }
    }
}

void draw_input(State* state, Object* object, Node* nodes, int view_x, int view_y, int spacing_zoom) {
    const Color* color;

    if (object->state == 0b00) {
        color = &wire_disconnected;
    } else if (object->state == 0b01) {
        color = &wire_off;
    } else if (object->state == 0b10) {
        color = &wire_on;
    } else if (object->state == 0b11) {
        color = &wire_conflicted;
    }

    draw_input_symbol(state, object->x, object->y, object->rotation, view_x, view_y, spacing_zoom, object->selected, *color, Color(0, 0, 0));
}

void draw_output_symbol(State* state, int x, int y, char rot, int view_x, int view_y, int zoom, bool selected, Color a, Color b) {
    const float sin_table[25] = {0, -0.258819, -0.5, -0.707107, -0.866025, -0.965926, -1, -0.965926, -0.866025, -0.707107, -0.5, -0.258819, 0, 0.258819, 0.5, 0.707107, 0.866025, 0.965926, 1, 0.965926, 0.866025, 0.707107, 0.5, 0.258819, 0};
    const float cos_table[25] = {1, 0.965926, 0.866025, 0.707107, 0.5, 0.258819, 0, -0.258819, -0.5, -0.707107, -0.866025, -0.965926, -1, -0.965926, -0.866025, -0.707107, -0.5, -0.258819, 0, 0.258819, 0.5, 0.707107, 0.866025, 0.965926, 1};
    int start_x = x * zoom + view_x;
    int start_y = y * zoom + view_y;
    int zoom_1_9 = zoom / 9.0;
    int zoom_1_6 = zoom / 6.0;
    int zoom_1_3 = zoom / 3.0;
    int zoom_2_3 = zoom * (2.0 / 3.0);

    if (rot == 0 || rot == 2) {
        if (rot == 0) {
            state->r.line(start_x, start_y, start_x + zoom_1_6, start_y - zoom_1_6, b);
            state->r.line(start_x + zoom_1_6, start_y - zoom_1_6, start_x + zoom_1_6, start_y - zoom_2_3, b);
            state->r.line(start_x, start_y, start_x - zoom_1_6, start_y - zoom_1_6, b);
            state->r.line(start_x - zoom_1_6, start_y - zoom_1_6, start_x - zoom_1_6, start_y - zoom_2_3, b);
            state->r.line(start_x + zoom_1_6, start_y - zoom_2_3, start_x - zoom_1_6, start_y - zoom_2_3, b);

            state->r.line(start_x, start_y, start_x, start_y - zoom_1_3, a);
            for (int i = 0; i < 24; i++) {
                state->r.triangle(start_x, start_y - zoom_1_3 - zoom_1_9, 
                                  start_x + sin_table[i] * zoom_1_9, start_y - zoom_1_3 - zoom_1_9 + cos_table[i] * zoom_1_9,
                                  start_x + sin_table[i + 1] * zoom_1_9, start_y - zoom_1_3 - zoom_1_9 + cos_table[i + 1] * zoom_1_9, a);
            }
            if (selected) { state->r.rect(start_x - zoom_1_6 - 3, start_y - zoom_2_3 - 3, start_x + zoom_1_6 + 3, start_y + 3, selection); }
        } else {
            state->r.line(start_x, start_y, start_x + zoom_1_6, start_y + zoom_1_6, b);
            state->r.line(start_x + zoom_1_6, start_y + zoom_1_6, start_x + zoom_1_6, start_y + zoom_2_3, b);
            state->r.line(start_x, start_y, start_x - zoom_1_6, start_y + zoom_1_6, b);
            state->r.line(start_x - zoom_1_6, start_y + zoom_1_6, start_x - zoom_1_6, start_y + zoom_2_3, b);
            state->r.line(start_x + zoom_1_6, start_y + zoom_2_3, start_x - zoom_1_6, start_y + zoom_2_3, b);

            state->r.line(start_x, start_y, start_x, start_y + zoom_1_3, a);
            for (int i = 0; i < 24; i++) {
                state->r.triangle(start_x, start_y + zoom_1_3 + zoom_1_9, 
                                  start_x + sin_table[i] * zoom_1_9, start_y + zoom_1_3 + zoom_1_9 + cos_table[i] * zoom_1_9,
                                  start_x + sin_table[i + 1] * zoom_1_9, start_y + zoom_1_3 + zoom_1_9 + cos_table[i + 1] * zoom_1_9, a);
            }
            if (selected) { state->r.rect(start_x - zoom_1_6 - 3, start_y - 3, start_x + zoom_1_6 + 3, start_y + zoom_2_3 + 3, selection); }
        }
    } else {
        if (rot == 1) {
            state->r.line(start_x, start_y, start_x + zoom_1_6, start_y + zoom_1_6, b);
            state->r.line(start_x + zoom_1_6, start_y + zoom_1_6, start_x + zoom_2_3, start_y + zoom_1_6, b);
            state->r.line(start_x, start_y, start_x + zoom_1_6, start_y - zoom_1_6, b);
            state->r.line(start_x + zoom_1_6, start_y - zoom_1_6, start_x + zoom_2_3, start_y - zoom_1_6, b);
            state->r.line(start_x + zoom_2_3, start_y + zoom_1_6, start_x + zoom_2_3, start_y - zoom_1_6, b);

            state->r.line(start_x, start_y, start_x + zoom_1_3, start_y, a);
            for (int i = 0; i < 24; i++) {
                state->r.triangle(start_x + zoom_1_3 + zoom_1_9, start_y, 
                                  start_x + zoom_1_3 + zoom_1_9 + sin_table[i] * zoom_1_9, start_y + cos_table[i] * zoom_1_9,
                                  start_x + zoom_1_3 + zoom_1_9 + sin_table[i + 1] * zoom_1_9, start_y + cos_table[i + 1] * zoom_1_9, a);
            }
            if (selected) { state->r.rect(start_x - 3, start_y - zoom_1_6 - 3, start_x + zoom_2_3 + 3, start_y + zoom_1_6 + 3, selection); }
        } else {
            state->r.line(start_x, start_y, start_x - zoom_1_6, start_y + zoom_1_6, b);
            state->r.line(start_x - zoom_1_6, start_y + zoom_1_6, start_x - zoom_2_3, start_y + zoom_1_6, b);
            state->r.line(start_x, start_y, start_x - zoom_1_6, start_y - zoom_1_6, b);
            state->r.line(start_x - zoom_1_6, start_y - zoom_1_6, start_x - zoom_2_3, start_y - zoom_1_6, b);
            state->r.line(start_x - zoom_2_3, start_y + zoom_1_6, start_x - zoom_2_3, start_y - zoom_1_6, b);

            state->r.line(start_x, start_y, start_x - zoom_1_3, start_y, a);
            for (int i = 0; i < 24; i++) {
                state->r.triangle(start_x - zoom_1_3 - zoom_1_9, start_y, 
                                  start_x - zoom_1_3 - zoom_1_9 + sin_table[i] * zoom_1_9, start_y + cos_table[i] * zoom_1_9,
                                  start_x - zoom_1_3 - zoom_1_9 + sin_table[i + 1] * zoom_1_9, start_y + cos_table[i + 1] * zoom_1_9, a);
            }
            if (selected) { state->r.rect(start_x - zoom_2_3 - 3, start_y - zoom_1_6 - 3, start_x + 3, start_y + zoom_1_6 + 3, selection); }
        }
    }
}

void draw_output(State* state, Object* object, Node* nodes, int view_x, int view_y, int spacing_zoom) {
    const Color* a_color;
    
    if (object->a == nullptr || object->a->state == 0b00) {
        a_color = &wire_disconnected;
    } else if (object->a->state == 0b01) {
        a_color = &wire_off;
    } else if (object->a->state == 0b10) {
        a_color = &wire_on;
    } else if (object->a->state == 0b11) {
        a_color = &wire_conflicted;
    }

    draw_output_symbol(state, object->x, object->y, object->rotation, view_x, view_y, spacing_zoom, object->selected, *a_color, Color(0, 0, 0));
}

void draw_objects_nodes(State* state, Object* objects, Node* nodes, int view_x, int view_y, int zoom) {
    Object* object = objects;
    while (object != nullptr) {
        switch (object->type) {
        case 'N': // NMOS
            draw_nmos(state, object, nodes, view_x, view_y, zoom);
            break;
        case 'P': // PMOS
            draw_pmos(state, object, nodes, view_x, view_y, zoom);
            break;
        case '+': // V+
            draw_vplus(state, object, nodes, view_x, view_y, zoom);
            break;
        case '-': // V-
            draw_vminus(state, object, nodes, view_x, view_y, zoom);
            break;
        case 'I': // Input
            draw_input(state, object, nodes, view_x, view_y, zoom);
            break;
        case 'O': // Output
            draw_output(state, object, nodes, view_x, view_y, zoom);
            break;
        default:
            break;
        }

        object = object->next;
    }

    Node* node = nodes;
    const Color* node_color;
    Wire* wire;
    while (node != nullptr) {
        if (node->state == 0b00) {
            node_color = &wire_disconnected;
        } else if (node->state == 0b01) {
            node_color = &wire_off;
        } else if (node->state == 0b10) {
            node_color = &wire_on;
        } else if (node->state == 0b11) {
            node_color = &wire_conflicted;
        }

        wire = node->wires;
        while (wire != nullptr) {
            state->r.rect(wire->x1 * zoom + view_x - 2, wire->y1 * zoom + view_y - 2, wire->x1 * zoom + view_x + 2, wire->y1 * zoom + view_y + 2, *node_color);
            state->r.rect(wire->x2 * zoom + view_x - 2, wire->y2 * zoom + view_y - 2, wire->x2 * zoom + view_x + 2, wire->y2 * zoom + view_y + 2, *node_color);
            state->r.line(wire->x1 * zoom + view_x, wire->y1 * zoom + view_y, wire->x2 * zoom + view_x, wire->y2 * zoom + view_y, *node_color);
            if (wire->selected) { state->r.rect(wire->x1 * zoom + view_x - 3, wire->y1 * zoom + view_y - 3, wire->x2 * zoom + view_x + 3, wire->y2 * zoom + view_y + 3, selection); }
            wire = wire->next;
        }

        node = node->next;
    }
}

bool point_in_rect(int x, int y, int x1, int y1, int x2, int y2) {
    if (x2 < x1) {
        int temp_x = x1;
        x1 = x2;
        x2 = temp_x;
    }
    if (y2 < y1) {
        int temp_y = y1;
        y1 = y2;
        y2 = temp_y;
    }
    return x > x1 && x < x2 && y > y1 && y < y2;
}

bool object_in_rect(Object* object, int rect_x1, int rect_y1, int rect_x2, int rect_y2, int view_x, int view_y, int zoom) {
    int x = object->x;
    int y = object->y;
    char rot = object->rotation;
    int x1, y1, x2, y2;
    if (object->type == 'N' || object->type == 'P') {
        x1 = x * zoom + view_x - 3;
        y1 = y * zoom + view_y - 3;
        if (rot == 0 || rot == 2) {
            x2 = x * zoom + (zoom * 2) + view_x + 3;
            y2 = y * zoom + zoom + view_y + 3;
        } else if (rot == 1 || rot == 3) {
            x2 = x * zoom + zoom + view_x + 3;
            y2 = y * zoom + (zoom * 2) + view_y + 3;
        }
    } else if (object->type == '+') {
        if (rot == 0) {
            x1 = x * zoom - zoom * 0.176777 + view_x - 3; // 0.176777 = sqrt(2) / 8
            y1 = y * zoom - zoom * 0.707107 + view_y - 3; // 0.707107 = sqrt(2) / 2
            x2 = x * zoom + zoom * 0.176777 + view_x + 3;
            y2 = y * zoom + view_y + 3;
        } else if (rot == 1) {
            x1 = x * zoom + view_x - 3;
            y1 = y * zoom - zoom * 0.176777 + view_y - 3; // 0.176777 = sqrt(2) / 8
            x2 = x * zoom + zoom * 0.707107 + view_x + 3; // 0.707107 = sqrt(2) / 2
            y2 = y * zoom + zoom * 0.176777 + view_y + 3;
        } else if (rot == 2) {
            x1 = x * zoom - zoom * 0.176777 + view_x - 3; // 0.176777 = sqrt(2) / 8
            y1 = y * zoom + view_y - 3;
            x2 = x * zoom + zoom * 0.176777 + view_x + 3;
            y2 = y * zoom + zoom * 0.707107 + view_y + 3; // 0.707107 = sqrt(2) / 2
        } else if (rot == 3) {
            x1 = x * zoom - zoom * 0.707107 + view_x - 3; // 0.707107 = sqrt(2) / 2
            y1 = y * zoom - zoom * 0.176777 + view_y - 3; // 0.176777 = sqrt(2) / 8
            x2 = x * zoom + view_x + 3;
            y2 = y * zoom + zoom * 0.176777 + view_y + 3;
        }
    } else if (object->type == '-') {
        if (rot == 0) {
            x1 = x * zoom - zoom * 0.176777 + view_x - 3; // 0.176777 = sqrt(2) / 8
            y1 = y * zoom - zoom / 2.0 - zoom * 0.141421 + view_y - 3; // 0.141421 = sqrt(2) / 10
            x2 = x * zoom + zoom * 0.176777 + view_x + 3;
            y2 = y * zoom + view_y + 3;
        } else if (rot == 1) {
            x1 = x * zoom + view_x - 3;
            y1 = y * zoom - zoom * 0.176777 + view_y - 3; // 0.176777 = sqrt(2) / 8
            x2 = x * zoom + zoom / 2.0 + zoom * 0.141421 + view_x + 3; // 0.141421 = sqrt(2) / 10
            y2 = y * zoom + zoom * 0.176777 + view_y + 3;
        } else if (rot == 2) {
            x1 = x * zoom - zoom * 0.176777 + view_x - 3; // 0.176777 = sqrt(2) / 8
            y1 = y * zoom + view_y - 3;
            x2 = x * zoom + zoom * 0.176777 + view_x + 3;
            y2 = y * zoom + zoom / 2.0 + zoom * 0.141421 + view_y + 3; // 0.141421 = sqrt(2) / 10
        } else if (rot == 3) {
            x1 = x * zoom - zoom / 2.0 - zoom * 0.141421 + view_x - 3; // 0.141421 = sqrt(2) / 10
            y1 = y * zoom - zoom * 0.176777 + view_y - 3; // 0.176777 = sqrt(2) / 8
            x2 = x * zoom + view_x + 3;
            y2 = y * zoom + zoom * 0.176777 + view_y + 3;
        }
    } else if (object->type == 'I' || object->type == 'O') {
        if (rot == 0) {
            x1 = x * zoom - zoom / 6.0 + view_x - 3;
            y1 = y * zoom - zoom * 2.0 / 3.0 + view_y - 3;
            x2 = x * zoom + zoom / 6.0 + view_x + 3;
            y2 = y * zoom + view_y + 3;
        } else if (rot == 1) {
            x1 = x * zoom + view_x - 3;
            y1 = y * zoom - zoom / 6.0 + view_y - 3;
            x2 = x * zoom + zoom * 2.0 / 3.0 + view_x + 3;
            y2 = y * zoom + zoom / 6.0 + view_y + 3;
        } else if (rot == 2) {
            x1 = x * zoom - zoom / 6.0 + view_x - 3;
            y1 = y * zoom + view_y - 3;
            x2 = x * zoom + zoom / 6.0 + view_x + 3;
            y2 = y * zoom + zoom * 2.0 / 3.0 + view_y + 3;
        } else if (rot == 3) {
            x1 = x * zoom - zoom * 2.0 / 3.0 + view_x - 3;
            y1 = y * zoom - zoom / 6.0 + view_y - 3;
            x2 = x * zoom + view_x + 3;
            y2 = y * zoom + zoom / 6.0 + view_y + 3;
        }
    }
    return point_in_rect(x1, y1, rect_x1, rect_y1, rect_x2, rect_y2) || 
           point_in_rect(x1, y2, rect_x1, rect_y1, rect_x2, rect_y2) || 
           point_in_rect(x2, y1, rect_x1, rect_y1, rect_x2, rect_y2) || 
           point_in_rect(x2, y2, rect_x1, rect_y1, rect_x2, rect_y2);
}

void update_inputs(State* state, Object* objects, int view_x, int view_y, int zoom) {
    if (state->i.get_mouse_button_pressed(MB_LEFT)) {
        int m_x = state->i.mouse.x;
        int m_y = state->i.mouse.y;
        Object* object = objects;
        while (object != nullptr) {
            if (object->type == 'I') {
                int start_x = object->x * zoom + view_x;
                int start_y = object->y * zoom + view_y;
                int zoom_2_3 = zoom * 2.0 / 3.0;
                int zoom_1_6 = zoom / 6.0;
                if (object->rotation == 0 && point_in_rect(m_x, m_y, start_x - zoom_1_6, start_y - zoom_2_3, start_x + zoom_1_6, start_y)) {
                    object->state++;
                } else if (object->rotation == 1 && point_in_rect(m_x, m_y, start_x, start_y - zoom_1_6, start_x + zoom_2_3, start_y + zoom_1_6)) {
                    object->state++;
                } else if (object->rotation == 2 && point_in_rect(m_x, m_y, start_x - zoom_1_6, start_y, start_x + zoom_1_6, start_y + zoom_2_3)) {
                    object->state++;
                } else if (object->rotation == 3 && point_in_rect(m_x, m_y, start_x - zoom_2_3, start_y - zoom_1_6, start_x, start_y + zoom_1_6)) {
                    object->state++;
                }
                if (object->state > 2) {
                    object->state = 0;
                }
            }
            object = object->next;
        }
    }
}

void select_selection(State* state, Object* objects, Node* nodes, int x1, int y1, int x2, int y2, int view_x, int view_y, int zoom) {
    Object* object = objects;
    while (object != nullptr) {
        object->selected = object_in_rect(object, x1, y1, x2, y2, view_x, view_y, zoom) || (object->selected && state->i.get_mod_key(MKC_SHIFT));
        object = object->next;
    }

    Node* node = nodes;
    Wire* wire;
    while (node != nullptr) {
        wire = node->wires;
        while (wire != nullptr) {
            wire->selected = point_in_rect(wire->x1 * zoom + view_x, wire->y1 * zoom + view_y, x1, y1, x2, y2) || 
                             point_in_rect(wire->x2 * zoom + view_x, wire->y2 * zoom + view_y, x1, y1, x2, y2) ||
                             (wire->selected && state->i.get_mod_key(MKC_SHIFT));
            wire = wire->next;
        }
        node = node->next;
    }
}

void update_draw_selection(State* state, Object* objects, Node* nodes, int* selection_x, int* selection_y, int view_x, int view_y, int zoom) {
    if (state->i.get_mouse_button_pressed(MB_RIGHT)) {
        *selection_x = state->i.mouse.x - view_x;
        *selection_y = state->i.mouse.y - view_y;
    }
    if (state->i.get_mouse_button(MB_RIGHT)) {
        state->r.rect(*selection_x + view_x, *selection_y + view_y, state->i.mouse.x, state->i.mouse.y, selection);
        select_selection(state, objects, nodes, *selection_x + view_x, *selection_y + view_y, state->i.mouse.x, state->i.mouse.y, view_x, view_y, zoom);
    }
}

inline int screenspace_to_gridspace(int n, int view, int zoom) {
    return round((float)(n - view) / (float)zoom);
}

bool wires_intersect(Wire* wire1, Wire* wire2) {
    // Wire a, b;
    // memcpy(&a, wire1, sizeof(Wire));
    // memcpy(&b, wire2, sizeof(Wire));
    // Wire temp;
    // if (b.x1 == b.x2 && a.y1 == a.y2) {
    //     return a.x1 <= b.x1 && a.x2 >= b.x1 && a.y1 >= b.y1 && a.y1 >= b.y2;
    // } else {
    //     temp = a;
    //     a = b;
    //     b = temp;
    //     return a.x1 <= b.x1 && a.x2 >= b.x1 && a.y1 >= b.y1 && a.y1 >= b.y2;
    // }
    return (wire1->x1 == wire2->x1 && wire1->y1 == wire2->y1) || (wire1->x2 == wire2->x2 && wire1->y2 == wire2->y2) ||
           (wire1->x1 == wire2->x2 && wire1->y1 == wire2->y2) || (wire1->x2 == wire2->x1 && wire1->y2 == wire2->y1);
}

struct WireSplit {
    Node* n;
    Wire* w1;
    Wire* w2;
    WireSplit* next;
};

void append_split(WireSplit** splits, WireSplit split) {
    WireSplit* next = (WireSplit*)malloc(sizeof(WireSplit));
    *next = split;
    if (*splits == nullptr) {
        *splits = next;
        return;
    }
    WireSplit* end;
    WireSplit* current = *splits;
    while (current != nullptr) {
        end = current;
        current = current->next;
    }
    end->next = next;
}

void split_nodes(Node** nodes) {
    Node* n1 = *nodes;
    Wire* w1;
    Wire* w2;
    WireSplit* splits = 0;
    WireSplit* split;
    bool already_split = false;
    bool should_split = false;
    Wire* split_wire;
    while (n1 != nullptr) { // find splits
        w1 = n1->wires;
        while (w1 != nullptr) {
            w2 = n1->wires;
            while (w2 != nullptr) {
                split = splits;
                while (split != nullptr) {
                    already_split = split->w1 == w2 || split->w2 == w2;
                    split = split->next;
                }
                if (w1 != w2 && !wires_intersect(w1, w2) && !already_split) {
                    should_split = true;
                    split_wire = w2;
                }
                already_split = false;
                w2 = w2->next;
            }
            if (should_split) {
                append_split(&splits, {n1, w1, split_wire, 0});
            }
            should_split = false;
            w1 = w1->next;
        }
        n1 = n1->next;
    }

    split = splits;
    while (split != nullptr) { // process splits
        delist_wire(&split->n->wires, split->w1);
        append_node(nodes, {0b00, split->w1, 0, 0});
        split = split->next;
    }

    split = splits;
    WireSplit* next;
    while (split != nullptr) {
        next = split->next;
        free(split);
        split = next;
    }
}

void combine_nodes(Node** nodes, Node* n1, Node* n2) {
    Wire* wire = n1->wires;
    Wire* n1_end;
    while (wire != nullptr) {
        n1_end = wire;
        wire = wire->next;
    }
    n1_end->next = n2->wires;
    n2->wires->prev = n1_end;
    delete_node(nodes, n2, false);
}

struct NodeMerge {
    Node* n1;
    Node* n2;
    NodeMerge* next;
};

void append_merge(NodeMerge** merges, NodeMerge merge) {
    NodeMerge* next = (NodeMerge*)malloc(sizeof(NodeMerge));
    *next = merge;
    if (*merges == nullptr) {
        *merges = next;
        return;
    }
    NodeMerge* end;
    NodeMerge* current = *merges;
    while (current != nullptr) {
        end = current;
        current = current->next;
    }
    end->next = next;
}

void merge_nodes(Node** nodes) {
    Node* n1 = *nodes;
    Wire* w1;
    Node* n2;
    Wire* w2;
    NodeMerge* merges = nullptr;
    NodeMerge* merge;
    bool already_merge = false;
    bool should_merge = false;

    while (n1 != nullptr) {
        n2 = *nodes;
        while (n2 != nullptr) {
            merge = merges;
            while (merge != nullptr) {
                if ((merge->n1 == n1 && merge->n2 == n2) || (merge->n1 == n2 && merge->n2 == n1)) {
                    already_merge = true;
                }
                merge = merge->next;
            }
            if (n1 != n2 && !already_merge) {
                w1 = n1->wires;
                while (w1 != nullptr) {
                    w2 = n2->wires;
                    while (w2 != nullptr) {
                        if (wires_intersect(w1, w2)) {
                            should_merge = true;
                        }
                        w2 = w2->next;
                    }
                    w1 = w1->next;
                }
                if (should_merge) {
                    append_merge(&merges, {n1, n2, 0});
                }
                should_merge = false;
            }
            already_merge = false;
            n2 = n2->next;
        }
        n1 = n1->next;
    }

    NodeMerge* merge2;
    Node* temp_n1;
    Node* temp_n2;
    merge = merges;
    while (merge != nullptr) {
        if (merge->n1 != merge->n2) {
            combine_nodes(nodes, merge->n1, merge->n2);
            merge2 = merges;
            temp_n1 = merge->n1;
            temp_n2 = merge->n2;
            while (merge2 != nullptr) {
                if (merge2->n1 == temp_n2) {
                    merge2->n1 = temp_n1;
                } else if (merge2->n2 == temp_n2) {
                    merge2->n2 = temp_n1;
                }
                merge2 = merge2->next;
            }
        }
        merge = merge->next;
    }

    merge = merges;
    NodeMerge* next;
    while (merge != nullptr) {
        next = merge->next;
        free(merge);
        merge = next;
    }
}

void connect_nodes(Object* objects, Node* nodes) {
    Object* object = objects;
    while (object != nullptr) {
        object->a = nullptr;
        object->b = nullptr;
        object->c = nullptr;
        object = object->next;
    }
    object = objects;
    Node* node;
    Wire* wire;
    while (object != nullptr) {
        node = nodes;
        while (node != nullptr) {
            wire = node->wires;
            while (wire != nullptr) {
                switch (object->type) {
                    case 'N': case 'P':
                        if (object->rotation == 0) {
                            if ((wire->x1 == object->x && wire->y1 == object->y + 1) || (wire->x2 == object->x && wire->y2 == object->y + 1)) {
                                object->a = node;
                            }
                            if ((wire->x1 == object->x + 1 && wire->y1 == object->y) || (wire->x2 == object->x + 1 && wire->y2 == object->y)) {
                                object->b = node;
                            }
                            if ((wire->x1 == object->x + 2 && wire->y1 == object->y + 1) || (wire->x2 == object->x + 2 && wire->y2 == object->y + 1)) {
                                object->c = node;
                            }
                        } else if (object->rotation == 1) {
                            if ((wire->x1 == object->x && wire->y1 == object->y) || (wire->x2 == object->x && wire->y2 == object->y)) {
                                object->a = node;
                            }
                            if ((wire->x1 == object->x + 1 && wire->y1 == object->y + 1) || (wire->x2 == object->x + 1 && wire->y2 == object->y + 1)) {
                                object->b = node;
                            }
                            if ((wire->x1 == object->x && wire->y1 == object->y + 2) || (wire->x2 == object->x && wire->y2 == object->y + 2)) {
                                object->c = node;
                            }
                        } else if (object->rotation == 2) {
                            if ((wire->x1 == object->x && wire->y1 == object->y) || (wire->x2 == object->x && wire->y2 == object->y)) {
                                object->a = node;
                            }
                            if ((wire->x1 == object->x + 1 && wire->y1 == object->y + 1) || (wire->x2 == object->x + 1 && wire->y2 == object->y + 1)) {
                                object->b = node;
                            }
                            if ((wire->x1 == object->x + 2 && wire->y1 == object->y) || (wire->x2 == object->x + 2 && wire->y2 == object->y)) {
                                object->c = node;
                            }
                        } else if (object->rotation == 3) {
                            if ((wire->x1 == object->x + 1 && wire->y1 == object->y) || (wire->x2 == object->x + 1 && wire->y2 == object->y)) {
                                object->a = node;
                            }
                            if ((wire->x1 == object->x && wire->y1 == object->y + 1) || (wire->x2 == object->x && wire->y2 == object->y + 1)) {
                                object->b = node;
                            }
                            if ((wire->x1 == object->x + 1 && wire->y1 == object->y + 2) || (wire->x2 == object->x + 1 && wire->y2 == object->y + 2)) {
                                object->c = node;
                            }
                        }
                    break;
                    case '+': case '-': case 'I': case 'O':
                        if ((wire->x1 == object->x && wire->y1 == object->y) || (wire->x2 == object->x && wire->y2 == object->y)) {
                            object->a = node;
                        }
                    break;
                }
                wire = wire->next;
            }
            node = node->next;
        }
        object = object->next;
    }
}

void update_nodes(State* state, Object* objects, Node** nodes) {
    split_nodes(nodes);
    merge_nodes(nodes);
    connect_nodes(objects, *nodes);
}

void drag_selection(State* state, Object* objects, Node** nodes, int* drag_x, int* drag_y, int* prev_drag_x, int* prev_drag_y, int m_x, int m_y) {
    if (state->i.get_mouse_button_pressed(MB_LEFT)) {
        *prev_drag_x = m_x;
        *prev_drag_y = m_y;
    }
    
    if (state->i.get_mouse_button(MB_LEFT)) {
        *drag_x = m_x;
        *drag_y = m_y;
        int delta_x = *drag_x - *prev_drag_x;
        int delta_y = *drag_y - *prev_drag_y;
        if (drag_x - prev_drag_x != 0 || drag_y - prev_drag_y != 0) {   
            Object* object = objects;
            while (object != nullptr) {
                if (object->selected) {
                    object->x += delta_x;
                    object->y += delta_y;
                }
                
                object = object->next;
            }

            Node* node = *nodes;
            Wire* wire;
            while (node != nullptr) {
                wire = node->wires;
                while (wire != nullptr) {
                    if (wire->selected) {
                        wire->x1 += delta_x;
                        wire->y1 += delta_y;
                        wire->x2 += delta_x;
                        wire->y2 += delta_y;
                    }

                    wire = wire->next;
                }

                node = node->next;
            }
        }

        *prev_drag_x = *drag_x;
        *prev_drag_y = *drag_y;
    }

    if (state->i.get_mouse_button_up(MB_LEFT)) {
        update_nodes(state, objects, nodes);
    }
}

void delete_selection(State* state, Object** objects, Node** nodes) {
    if (state->i.get_key_pressed(KC_BACKSPACE)) {
        Object* object = *objects;
        Object* next_object;
        while (object != nullptr) {
            next_object = object->next;
            
            if (object->selected) {
                delete_object(objects, object);
            }
            object = next_object;
        }

        Node* node = *nodes;
        Node* next_node;
        Wire* wire; 
        Wire* next_wire;
        while (node != nullptr) {
            next_node = node->next;
            
            wire = node->wires;
            while (wire != nullptr) {
                next_wire = wire->next;

                if (wire->selected) {
                    delete_wire(&node->wires, wire);
                }

                wire = next_wire;
            }

            if (node->wires == nullptr) {
                delete_node(nodes, node, true);
            }

            node = next_node;
        }
        update_nodes(state, *objects, nodes);
    }
}

void place_object(State* state, Object** objects, Node** nodes, int m_x, int m_y) {
    if (state->i.get_key_pressed(KC_1)) {
        append_object(objects, {'N', false, m_x, m_y, 0, nullptr, nullptr, nullptr, false, 0, 0});
        update_nodes(state, *objects, nodes);
    } else if (state->i.get_key_pressed(KC_2)) {
        append_object(objects, {'P', false, m_x, m_y, 0, nullptr, nullptr, nullptr, false, 0, 0});
        update_nodes(state, *objects, nodes);
    } else if (state->i.get_key_pressed(KC_3)) {
        append_object(objects, {'+', false, m_x, m_y, 0, nullptr, nullptr, nullptr, false, 0, 0});
        update_nodes(state, *objects, nodes);
    } else if (state->i.get_key_pressed(KC_4)) {
        append_object(objects, {'-', false, m_x, m_y, 0, nullptr, nullptr, nullptr, false, 0, 0});
        update_nodes(state, *objects, nodes);
    } else if (state->i.get_key_pressed(KC_5)) {
        append_object(objects, {'I', false, m_x, m_y, 0, nullptr, nullptr, nullptr, false, 0, 0});
        update_nodes(state, *objects, nodes);
    } else if (state->i.get_key_pressed(KC_6)) {
        append_object(objects, {'O', false, m_x, m_y, 0, nullptr, nullptr, nullptr, false, 0, 0});
        update_nodes(state, *objects, nodes);
    }    
}

void rotate_selection(State* state, Object* objects, Node** nodes) { // maybe implement actual selection rotation later
    if (state->i.get_key_pressed(KC_R)) {
        Object* object = objects;

        while (object != nullptr) {
            if (object->selected) {
                object->rotation++;
                if (object->rotation == 4) {
                    object->rotation = 0;
                }
            }
            object = object->next;
        }
        update_nodes(state, objects, nodes);
    }
}

void place_wire(State* state, Object* objects, Node** nodes, int m_x, int m_y) {
    static Wire current = {0};
    if (state->i.get_key_pressed(KC_T)) {
        current.x1 = m_x;
        current.y1 = m_y;
    }
    if (state->i.get_key_pressed(KC_G)) {
        if (abs(current.x1 - m_x) > abs(current.y1 - m_y)) {
            current.x2 = m_x;
            current.y2 = current.y1;
        } else {
            current.x2 = current.x1;
            current.y2 = m_y;
        }
        Wire* wire = 0;
        append_wire(&wire, current);
        append_node(nodes, {0b00, wire, 0, 0});
        update_nodes(state, objects, nodes);
    }
}

/*
NMOS truth table:
    b g | s
    -------
    0 0 | x
    0 1 |
    1 0 | 0
    1 1 |
*/


char nmos(char b, char g) {
    return (b == 0b01 && g == 0b10) ? 0b01 : 0b00;
}

char pmos(char b, char g) {
    return (b == 0b10 && (g == 0b00 || g == 0b01)) ? 0b10 : 0b00;
}

void sim_step(State* state, Object* objects, Node* nodes) {
    Node* node = nodes;
    while (node != nullptr) {
        node->state = 0;
        node = node->next;
    }

    Object* object = objects;
    char a_state;
    while (object != nullptr) {
        switch (object->type) {
            case 'N':
                if (object->a == nullptr && object->c != nullptr) {
                    object->state = nmos(object->c->state, object->b->state);
                } else if (object->a != nullptr && object->c == nullptr) {
                    object->state = nmos(object->a->state, object->b->state);
                } else if (object->a != nullptr && object->c != nullptr) {
                    a_state = object->a->state;
                    object->a->state |= nmos(object->c->state, object->b->state);
                    object->c->state |= nmos(a_state, object->b->state);
                }
                break;
            case 'P':
                if (object->a == nullptr && object->c != nullptr) {
                    object->state = pmos(object->c->state, object->b->state);
                } else if (object->a != nullptr && object->c == nullptr) {
                    object->state = pmos(object->a->state, object->b->state);
                } else if (object->a != nullptr && object->c != nullptr) {
                    a_state = object->a->state;
                    object->a->state |= pmos(object->c->state, object->b->state);
                    object->c->state |= pmos(a_state, object->b->state);
                }
                break;
            case '+':
                if (object->a != nullptr) {
                    object->a->state |= 0b10;
                }
                break;
            case '-':
                if (object->a != nullptr) {
                    object->a->state |= 0b01;
                }
                break;
            case 'I':
                if (object->a != nullptr) {
                    object->a->state |= object->state;
                }
                default: break;
        }
        object = object->next;
    }
}
    
void draw_update_sim_controls(State* state, bool* loop, bool* step) {
    state->r.rect(10, 10, 74, 42, Color(49, 49, 49));
    if (*loop) {
        state->r.rect(15, 15, 42, 37, wire_on);
    } else {
        state->r.rect(15, 15, 42, 37, wire_off);
    }
    int m_x = state->i.mouse.x;
    int m_y = state->i.mouse.y;
    if (state->i.get_mouse_button_pressed(MB_LEFT) && m_x > 15 && m_x < 42 && m_y > 15 && m_y < 37) {
        *loop = !*loop;
    }
    if (state->i.get_mouse_button_pressed(MB_LEFT) && m_x > 47 && m_x < 69 && m_y > 15 && m_y < 37) {
        *step = true;
    }
    if (*step) {
        state->r.rect(47, 15, 69, 37, wire_on);
    } else {
        state->r.rect(47, 15, 69, 37, wire_off);
    }
}

int main() {
    State state = State("test", 640, 480, 120, WINDOW_RESIZEABLE | INPUT_MULTI_THREADED | ELEMENTS_ENABLE);

    const int grid_spacing = 100;

    Wire* wires1 = 0;
    append_wire(&wires1, {1, 3, 2, 3, false, 0, 0});
    Wire* wires2 = 0;
    append_wire(&wires2, {3, 1, 3, 2, false, 0, 0});
    Wire* wires3 = 0;
    append_wire(&wires3, {4, 3, 5, 3, false, 0, 0});
    Node* nodes = 0;
    append_node(&nodes, {0b00, wires1, 0, 0});
    append_node(&nodes, {0b01, wires2, 0, 0});
    append_node(&nodes, {0b10, wires3, 0, 0});
    Object* objects = 0;
    append_object(&objects, {'P', false, 2, 2, 3, nodes, nodes->next, nodes->next->next, false, 0, 0});

    
    int view_x = 0;
    int view_y = 0;
    float zoom = 1;

    int selection_x = 0;
    int selection_y = 0;

    int prev_drag_x = 0;
    int prev_drag_y = 0;
    int drag_x = 0;
    int drag_y = 0;

    int gs_mouse_x = 0;
    int gs_mouse_y = 0;

    bool loop = false;
    bool step = false;

    while (!state.quit) {
        state.start_frame();

        gs_mouse_x = screenspace_to_gridspace(state.i.mouse.x, view_x, grid_spacing * zoom);
        gs_mouse_y = screenspace_to_gridspace(state.i.mouse.y, view_y, grid_spacing * zoom);

        if (state.i.get_key_pressed(KC_Q)) {
            zoom /= 2.0;
        }
        if (state.i.get_key_pressed(KC_E)) {
            zoom *= 2.0;
        }
        if (state.i.get_key(KC_W)) {
            view_y += 1;
        }
        if (state.i.get_key(KC_S)) {
            view_y -= 1;
        }
        if (state.i.get_key(KC_A)) {
            view_x += 1;
        }
        if (state.i.get_key(KC_D)) {
            view_x -= 1;
        }
        draw_grid(&state, view_x, view_y, zoom, grid_spacing);
        draw_objects_nodes(&state, objects, nodes, view_x, view_y, grid_spacing * zoom);
        update_draw_selection(&state, objects, nodes, &selection_x, &selection_y, view_x, view_y, grid_spacing * zoom);
        update_inputs(&state, objects, view_x, view_y, grid_spacing * zoom);
        drag_selection(&state, objects, &nodes, &drag_x, &drag_y, &prev_drag_x, &prev_drag_y, gs_mouse_x, gs_mouse_y);
        delete_selection(&state, &objects, &nodes);
        rotate_selection(&state, objects, &nodes);
        place_object(&state, &objects, &nodes, gs_mouse_x, gs_mouse_y);
        place_wire(&state, objects, &nodes, gs_mouse_x, gs_mouse_y);
        draw_update_sim_controls(&state, &loop, &step);
        if (loop || step) {
            sim_step(&state, objects, nodes);
        }
        if (step) {
            step = false;
        }
        Object* object = objects;
        printf("objects: ");
        while (object != nullptr) {
            int a = (object->a != nullptr) ? object->a->state : -1;
            int b = (object->b != nullptr) ? object->b->state : -1;
            int c = (object->c != nullptr) ? object->c->state : -1;
            printf("(%c, (%i, %i), %i, %i, (%i, %i, %i), %i)  ", object->type, object->x, object->y, object->rotation, object->state, a, b, c, object->selected);
            object = object->next;
        }
        printf("\n");
        state.r.rect(gs_mouse_x * grid_spacing * zoom + view_x - 5, gs_mouse_y * grid_spacing * zoom + view_y - 5, gs_mouse_x * grid_spacing * zoom + view_x + 5, gs_mouse_y * grid_spacing * zoom + view_y + 5, Color(255, 0, 0, 35));

        state.end_frame();
    }

    free_objects(objects);
    free_nodes(nodes);

    state.destroy_elements();
}