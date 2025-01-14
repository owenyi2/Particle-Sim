#include <stdlib.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

#include "arena.h"

#define PARTICLE_SIZE 3

typedef struct Particle {
    Vector2 s;
    Vector2 v;
    int last_contact; // -1 for walls
} Particle;

Particle particle_random(int width, int height, float max_speed) {
    Particle particle; 
    particle = (Particle) { .s = (Vector2) { GetRandomValue(0, width), GetRandomValue(0, height) }, (Vector2) { GetRandomValue(-max_speed, max_speed), GetRandomValue(-max_speed, max_speed) }, -1};
    return particle;
}

int main(void)
{
    DynamicArena particle_arena = da_new_with_capacity(sizeof(Particle), 4);

    bool exit_window = false;
    const int screenWidth = 640;
    const int screenHeight = 360;

    float total_kinetic_energy;
    Vector2 total_momentum;
    Vector2 container_momentum = Vector2Zero();

    InitWindow(screenWidth, screenHeight, "hello");

    // SetTargetFPS(360);              

    for (int i = 0; i < 400; i++){
        Particle particle = particle_random(screenWidth, screenHeight, 500);
        da_push(&particle_arena, &particle);
    }    

    while (!exit_window)    
    {
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_W)) {
            exit_window = true; 
        } else if (WindowShouldClose()){
            exit_window = true;
        }

        float dt = GetFrameTime();
 
        for (int i = 0; i < particle_arena.len; i++) {
            for (int j = 0; j < particle_arena.len; j++) {
                if (i <= j) {
                    continue;
                }
                Particle* particle_1;
                Particle* particle_2;
                if (
                    ((particle_1 = (Particle*) da_get(&particle_arena, i)) == NULL) || 
                    ((particle_2 = (Particle*) da_get(&particle_arena, j)) == NULL) 
                ) {
                    fprintf(stderr, "Uh oh\n");
                    exit(1);

                }


                if (!(Vector2DistanceSqr(particle_1->s, particle_2->s) < 4*PARTICLE_SIZE*PARTICLE_SIZE)) {
                    continue;
                }
                if ((particle_1->last_contact == j) || (particle_2->last_contact == i)) {
                    continue;
                }
                particle_1->last_contact = j;
                particle_2->last_contact = i;
                Vector2 new_particle_1_v = Vector2Subtract(particle_1->v, Vector2Scale(Vector2Subtract(particle_1->s, particle_2->s), Vector2DotProduct(Vector2Subtract(particle_1->v, particle_2->v), Vector2Subtract(particle_1->s, particle_2->s)) / Vector2LengthSqr(Vector2Subtract(particle_1->s, particle_2->s))));
                particle_2->v = Vector2Subtract(particle_2->v, Vector2Scale(Vector2Subtract(particle_2->s, particle_1->s), Vector2DotProduct(Vector2Subtract(particle_2->v, particle_1->v), Vector2Subtract(particle_2->s, particle_1->s)) / Vector2LengthSqr(Vector2Subtract(particle_2->s, particle_1->s))));
                particle_1->v = new_particle_1_v;
            }
        }
        
        total_kinetic_energy = 0;
        total_momentum = container_momentum;

        for (int i = 0; i < particle_arena.len; i++) {
            Particle* particle; 
            if ((particle = (Particle*) da_get(&particle_arena, i)) == NULL) {
                fprintf(stderr, "Uh oh\n");
                exit(1);
            }
            
            particle->s.x += particle->v.x * dt; 
            particle->s.y += particle->v.y * dt; 

            if (particle->s.x > screenWidth) {
                container_momentum = Vector2Add(container_momentum, (Vector2) { 2 * particle->v.x, 0 });
                particle->s.x = screenWidth;
                particle->v.x = -particle->v.x;
                particle->last_contact = -1;
            }
            if (particle->s.x < 0) {
                container_momentum = Vector2Add(container_momentum, (Vector2) { 2 * particle->v.x, 0 });
                particle->s.x = 0;
                particle->v.x = -particle->v.x;
                particle->last_contact = -1;
            }
            if (particle->s.y > screenHeight) {
                container_momentum = Vector2Add(container_momentum, (Vector2) { 0, 2 * particle->v.y });
                particle->s.y = screenHeight;
                particle->v.y = -particle->v.y;
                particle->last_contact = -1;
            }
            if (particle->s.y < 0) {
                container_momentum = Vector2Add(container_momentum, (Vector2) { 0, 2 * particle->v.y });
                particle->s.y = 0;
                particle->v.y = -particle->v.y;
                particle->last_contact = -1;
            }

            total_kinetic_energy += 0.5 * Vector2LengthSqr(particle->v);
            total_momentum = Vector2Add(total_momentum, particle->v);
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            DrawText(TextFormat("FPS: %d", GetFPS()), 0, 0, 20, GRAY);
            DrawText(TextFormat("Total KE: %.2f", total_kinetic_energy), 0, 20, 20, GRAY);
            DrawText(TextFormat("Total p: (%.2f, %.2f)", total_momentum.x, total_momentum.y), 0, 40, 20, GRAY);

            for (int i = 0; i < particle_arena.len; i++) {
                Particle* particle = (Particle*) da_get(&particle_arena, i);
                DrawCircle(particle->s.x, particle->s.y, PARTICLE_SIZE, RED);
            }

        EndDrawing();
    }

    CloseWindow();        

    return 0;
}
