#include "collision.h"

vec2
closest_point_aabb0(RECT *r, vec2 *p)
{
    return (vec2){
        .vx = CLAMP(p->vx, r->x, r->x + r->w),
        .vy = CLAMP(p->vy, r->y, r->y + r->h),
    };
}

vec2
ptdiff0(vec2 *a0, vec2 *b0)
{
    return (vec2) {
        .vx = b0->vx - a0->vx,
        .vy = b0->vy - a0->vy
    };
}

int32_t
sqdist0(vec2 *a0, vec2 *b0, vec2 *r_delta)
{
    vec2 dpt = ptdiff0(a0, b0);
    if(r_delta) (*r_delta) = dpt;
    return (dpt.vx * dpt.vx) + (dpt.vy * dpt.vy);
}

uint8_t
test_circ0_aabb0(vec2 *circp, int32_t r, RECT *aabb, vec2 *r_delta)
{
    vec2 closest = closest_point_aabb0(aabb, circp);
    int32_t sqdist = sqdist0(circp, &closest, r_delta);
    uint8_t collided = sqdist <= (r * r);
    return collided;
}


// Casos de colisão:
// 1. Bola está parcialmente fora da caixa (pelo menos uma coordenada
//    em closest é diferente do centro da bola);
//    - Recuperar os deltas da comparação;
//    - Com os deltas, definir o sentido da reflexão da velocidade;
//      - Delta positivo: Posição da bola é menor naquela direção.
//      - Delta negativo: Posição da bola é maior naquela direção.
//    - Calcular compensação de coordenadas relativo ao canto sup. esq.
//      da caixa.
//      - Pegar o ponto mais próximo;
//      - Calcular a diferença no eixo que não está igual;
//      - Subtrair esse valor do raio da esfera.
//      - Soma/subtração vai se dar de acordo com o sentido da colisão
//        (sinal oposto ao delta da comparação naquele eixo).
// 2. Bola está completamente dentro da caixa (ambas as coordenadas
//    são iguais a closest);
//    - Calcular os deltas com relação ao centro da caixa;
//    - Com os deltas, definir o sentido da reflexão da velocidade;
//    - Calcular compensação de coordenadas relativo ao canto sup. esq.;
//      da caixa.
uint8_t
collision_ball_box(vec2 *ballpos, int32_t r,
                   RECT *box,
                   vec2 *r_mov, vec2 *r_pos)
{
    vec2 delta;
    if(!test_circ0_aabb0(ballpos, r, box, &delta)) return 0;

    (*r_mov) = (*r_pos) = (vec2){ .vx = 0, .vy = 0 };

    // If ball is completely inside of box, recalculate delta
    // based on exact middle of box
    if((delta.vx == 0) && (delta.vy == 0)) {
        vec2 hbox = {
            .vx = box->x + (box->w >> 1),
            .vy = box->y + (box->h >> 1)
        };
        delta = ptdiff0(ballpos, &hbox);
    }
    // Otherwise, ball is partially out of the box.
    // If so, don't touch delta

    if(delta.vx > 0) {
        // Ball is at left of box
        r_pos->vx = box->x - r;
        r_mov->vx = -1;
    } else if(delta.vx < 0) {
        // Ball is at right of box
        r_pos->vx = box->x + box->w + r;
        r_mov->vx = 1;
    }

    if(delta.vy > 0) {
        // Ball is at top of box
        r_pos->vy = box->y - r;
        r_mov->vy = -1;
    } else if(delta.vy < 0) {
        // Ball is at bottom of box
        r_pos->vy = box->y + box->h + r;
        r_mov->vy = 1;
    }

    return 1;
}
