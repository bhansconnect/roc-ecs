# Roc ECS

This is just some musings to test the viability of Entity Component Systems in Roc.
The main goal of this repo are to implement a simple ECS example in 3 ways:
 - Raw C++ to test max performance
 - Raw Roc to test performance loss due to Roc
 - Roc via basic ECS library to measure the overhead of making it generic in Roc (might need to re-evaluate once abilities exist)


The examples will all be based on the same C++ for rendering just with different game managment.
The example will be able to be test with varying numbers of entities, but always with an explicit cap to max entities.
