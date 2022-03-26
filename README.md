# Roc ECS

This is just some musings to test the viability of Entity Component Systems in Roc.
The main goal of this repo are to implement a simple ECS example in 3 ways:
 - Raw C++ to test max performance
 - Raw Roc to test performance loss due to Roc
 - Roc via basic ECS library to measure the overhead of making it generic in Roc (might need to re-evaluate once abilities exist)


The examples will all be based on the same C++ for rendering just with different game managment.
The example will be able to be test with varying numbers of entities, but always with an explicit cap to max entities.

The target example: Fireworks and explosions (though rendering will just be circles with varying size, color and opacity).
 - Every N time a firework is spawned (maybe to keep a target particle count).
 - A firework goes up until it runs out of life
 - The firework explodes releasing M particles
 - A firework has a rare chance to create different color particles
 - Particles have an initial velocity (derived from a random direction and strength, maybe roughly circular)
 - Particles are effected by gravity
 - Particles have a life time and a color
 - The color fades as it gets closer to the end of its life
 - time will be measured in frames so that performance is very visible once you pass the limit for consistent FPS


Tags
 - FeelsGravity -> If the entity is effected by gravity

Components
 - DeathTime -> frame that this entity dies
 - Fades -> the minimum and rate of fading for each color component
 - Explodes -> number of particles to generate
 - Graphics -> color and radius of the entity
 - Position -> x y
 - Velocity -> dx dy

Systems
 - Spawn -> creates a firework based on the current spawn rate
 - Death -> Marks entities dead 
 - Explosion -> spawns explosion particles for newly dead entities
 - Fade -> updates the color based on rates.
 - Gravity -> lowers y velocity of an entity if it is effected
 - Move -> applies velocity to a position
 - Graphics -> creates a list of position, color, radius to be drawn
 

