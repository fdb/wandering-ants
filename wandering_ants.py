# Wandering Ants algorithm. Run in NodeBox 1.

from itertools import combinations
from math import pi, sin, cos
from random import seed

size(1920, 1080)
speed(60)
seed(42)

ANT_SPEED = 2.0
ANT_SPAWN_DISTANCE_SQUARED = 36
ANT_COOL_DOWN_TIME = 20

class Ant:
    def __init__(self):
        self.x = WIDTH / 4 + random(WIDTH/2)
        self.y = HEIGHT / 4 + random(HEIGHT / 2)
        self.cool_down = 0
        self.set_random_direction()
        self.trail = []

    def set_random_direction(self):
        dir = random(pi * 2)
        self.dx = cos(dir) * ANT_SPEED
        self.dy = sin(dir) * ANT_SPEED        
                    
def wander(ants):
    for ant in ants:        
        ant.x += ant.dx
        ant.y += ant.dy
        ant.trail.append([ant.x, ant.y])
        if random() > 0.9:
            ant.set_random_direction()
            
def cool_down_ants(ants):
    for ant in ants:
        ant.cool_down -= 1

def draw_trails(ants):
    nofill()
    stroke(0.7)
    beginpath()
    for ant in ants:
        for i, (x, y) in enumerate(ant.trail):
            if i == 0:
                moveto(x, y)
            else:
                lineto(x, y)            
    endpath()
    
def spawn_if_close(ants):
    new_ants = []
    for a, b in combinations(ants, 2):
        if a.cool_down <= 0 and b.cool_down <= 0:
            dsq = (a.x - b.x) ** 2 + (a.y - b.y) ** 2
            if dsq < ANT_SPAWN_DISTANCE_SQUARED:
                ant = Ant()
                ant.x = (a.x + b.x) / 2
                ant.y = (a.y + b.y) / 2
                new_ants.append(ant)
                a.cool_down = ANT_COOL_DOWN_TIME
                b.cool_down = ANT_COOL_DOWN_TIME
                ant.cool_down = ANT_COOL_DOWN_TIME
    return new_ants
        
def draw_ants(ants, sz=4):
    nostroke()
    fill(0)
    p = BezierPath()
    for ant in ants:
        p.oval(ant.x - sz, ant.y - sz, sz * 2, sz * 2)
    drawpath(p)
    
ants = []    

def setup():
    for i in range(40):
        ants.append(Ant())
    
def draw():
    autoclosepath(False)    
    wander(ants)
    cool_down_ants(ants)
    new_ants = spawn_if_close(ants)
    draw_trails(ants)
    draw_ants(ants)
    draw_ants(new_ants, 20)
    ants.extend(new_ants)
    
    
    
