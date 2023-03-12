import sys, pygame, random
import pygame.gfxdraw
random.seed()
pygame.init()
size = width, height = 896, 512
floor = 512-96 #constant to stop gravity

screen = pygame.display.set_mode(size)
title = pygame.display.set_caption("2D Adventure")
#background
background = [pygame.image.load(f'assets/sprite/scene/graveyard/bg/bg{i}.png')for i in range(1,4)]
background = [pygame.transform.smoothscale(sprite,size)for sprite in background]
#pygame.transform.scale(pygame.image.load("assets/sprite/scene/graveyard/bg/bg2.png"),size)
bglevel = 0
#tiles
ground = []
for i in range(0,16):#mass load tiles
    ground.append(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/scene/graveyard/tiles/tile"+str(i+1)+".png")))
#object
objects = []
for i in range(0,10):#mass load objects
    objects.append(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/scene/graveyard/object/obj"+str(i+1)+".png")))
#animations
sScale = 0.4
sIdle = [pygame.image.load(f'assets/sprite/enemy/skeleton/idle/idle{i}.png')for i in range(1,13)]
sIdle = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(224*sScale,364*sScale))for sprite in sIdle]
sRun = [pygame.image.load(f'assets/sprite/enemy/skeleton/walk/walk{i}.png')for i in range(1,9)]
sRun = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(224*sScale,364*sScale))for sprite in sRun]
sAttack = [pygame.image.load(f'assets/sprite/enemy/skeleton/attack/attack{i}.png')for i in range(1,9)]
sAttack = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(224*sScale,364*sScale))for sprite in sAttack]
sSpawn = [pygame.image.load(f'assets/sprite/enemy/skeleton/appear/appear{i}.png')for i in range(1,11)]
sSpawn = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(224*sScale,364*sScale))for sprite in sSpawn]
sDead = [pygame.image.load(f'assets/sprite/enemy/skeleton/dead/dead{i}.png')for i in range(1,9)]
sDead = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(224*sScale,364*sScale))for sprite in sDead]
running = True
#font/stat
heart = pygame.transform.scale(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/stat/heart.png")),(32,32))
coin = pygame.transform.scale(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/stat/coin.png")),(32,32))
skull = [pygame.image.load(f'assets/sprite/stat/skull{i}.png')for i in range(1,5)]
skull = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(32,32))for sprite in skull]
armor = [pygame.image.load(f'assets/sprite/stat/armor{i}.png')for i in range(1,7)]
armor = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(32,32))for sprite in armor]
sword = [pygame.image.load(f'assets/sprite/stat/sword{i}.png')for i in range(1,7)]
sword = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(32,32))for sprite in sword]
gems = [pygame.image.load(f'assets/sprite/stat/gem{i}.png')for i in range(1,11)]#elite achievements, try to get all 10 of them!
gems = [pygame.transform.scale(pygame.Surface.convert_alpha(sprite),(32,32))for sprite in gems]
gemCount = 10
scoreR = [1000000,10000000,20000000]
killR = [250,500,750]
moneyR = [50000,100000,150000]
# scoreR = [100000,1000000,2000000]
# killR = [25,50,75]
# moneyR = [5000,10000,15000]
swordCost = [100,500,1000,5000,10000]
swordStr = [1,1.5,2,3,4,5]
armorCost = [150,750,1500,7500,15000]
swordLevel = 0
armorLevel = 0
font = pygame.font.Font("assets/font/rainyhearts.ttf",40)
fontsmol = pygame.font.Font("assets/font/rainyhearts.ttf",30)
fontverysmol = pygame.font.Font("assets/font/rainyhearts.ttf",20)
font1 = pygame.font.Font("assets/font/rainyhearts.ttf",50)
gameover = font1.render("Game Over!",True,(255,255,255))
padding = 30
#stuff
red = (255,0,0)
yellow = (255,255,0)
green = (0,255,0)
white = (255,255,255)
black = (0,0,0)

#character
scale = 0.3
playerOffset = (0,64)
combo = 0
score = 0
killcount = 0
scoretextscale = 1 #the bigger it is, the more hyped up it gets
class character:
    animation_name = {0:"idle",1:"run",2:"jump",3:"attack",4:"dead"}
    def __init__(self):
        self.maxhealth = 5
        self.health = 5
        self.str = 1#literal multipler to enemy
        self.dr = 1#literal multiplier to damage taken
        self.state = 0#0=idle
        self.clock = 0
        self.frame = 0
        self.combodef = 0#combo protection %
        self.money = 0#dosh
        self.pos = (padding,floor)#reference to top left corner
        print(font.size("{}/{}".format(self.health,self.maxhealth)))
        #get all his sprites in
        self.idle = []
        self.walk = []
        self.jump = []
        self.attack = []
        self.dead = []
        for i in range(10):
            self.idle.append(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/character/ninja/idle/Idle__00{}.png".format(i))))
            self.idle[i] = pygame.transform.smoothscale(self.idle[i],(290*scale,500*scale))
            self.walk.append(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/character/ninja/run/Run__00{}.png".format(i))))
            self.walk[i] = pygame.transform.smoothscale(self.walk[i],(290*scale,500*scale))
            self.dead.append(pygame.transform.smoothscale(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/character/ninja/dead/Dead__00{}.png".format(i))),(578*(500*scale/599),500*scale)))
            self.jump.append(pygame.transform.smoothscale(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/character/ninja/jump/Jump__00{}.png".format(i))),(290*scale,500*scale)))
            self.attack.append(pygame.transform.smoothscale(pygame.Surface.convert_alpha(pygame.image.load("assets/sprite/character/ninja/attack/Attack__00{}.png".format(i))),(524*(500*scale/565),500*scale)))
        self.hitbox = self.idle[0].get_rect()#a better way of doing hitboxes lol
    def getLifeText(self):
        return fontsmol.render("{:.1f}/{:.0f}".format(self.health,self.maxhealth),True,(255,0,0))
    def renderScoreText(self):
        global scoretextscale
        #update the scale
        scoretextscale = max(1,scoretextscale-(0.1/15))
        #calculate stuff
        scoreStr = str(score).rjust(7,'0')
        comboStr = ("X"+str(combo)).rjust(7)
        pad = 4
        bgW = (font.size(scoreStr)[0])
        bgH = (font.size(scoreStr)[1])*2
        bgOH = (font.size(scoreStr)[1])*(scoretextscale-1)
        surface = pygame.Surface((bgW+pad+10,bgH+pad-3))
        #actually draw stuff
        pygame.draw.rect(surface,white,(0,0,bgW+pad+10,bgH+pad-3),width=0)
        surface.set_alpha(32)
        #bg layer
        screen.blit(surface,(width-padding-bgW-pad-2,padding))
        #layer 1
        screen.blit(pygame.transform.smoothscale_by(font.render(scoreStr,True,(128,128,128)),(1,scoretextscale)),(width-padding-(font.size(scoreStr)[0])+2,padding+2-bgOH))
        #primary layer
        screen.blit(pygame.transform.smoothscale_by(font.render(scoreStr,True,white),(1,scoretextscale)),(width-padding-(font.size(scoreStr)[0]),padding-bgOH))
        #screen.blit(font.render(scoreStr,True,(255,255,255)),(width-padding-(font.size(scoreStr)[0]),padding))
        #combo
        screen.blit(font.render(comboStr,True,yellow),(width-padding-(font.size(comboStr)[0]),padding+32))
    def animate(self):
        (pW,pH)=self.pos
        pH -= 707*scale
        pH += playerOffset[1]
        pW += playerOffset[0]
        if self.state == 0:
            self.hitbox = self.idle[self.frame].get_rect(topleft=(pW,pH))
            screen.blit(self.idle[self.frame],(pW,pH))
        elif self.state == 1:
            pH+=5
            if left_key:
                self.walk[self.frame] = pygame.transform.flip(self.walk[self.frame],True,False)
            self.hitbox = self.walk[self.frame].get_rect(topleft=(pW,pH))
            screen.blit(self.walk[self.frame],(pW,pH))
            if left_key:
                self.walk[self.frame] = pygame.transform.flip(self.walk[self.frame],True,False)
        elif self.state == 2:
            pH+=5
            self.frame = min(self.frame,9)
            self.hitbox = self.jump[self.frame].get_rect(topleft=(pW,pH))
            screen.blit(self.jump[self.frame],(pW,pH))
        elif self.state == 3:
            pH+=15
            self.frame = min(self.frame,9)
            self.hitbox = self.attack[self.frame].get_rect(topleft=(pW,pH))
            screen.blit(self.attack[self.frame],(pW,pH))
        elif self.state == 4:
            pH+=15
            self.frame = min(self.frame,9)
            screen.blit(self.dead[self.frame],(pW,pH))
        self.clock += 1
        if(self.clock%6 == 0):
            self.frame += 1
            if(self.state != 4):self.frame %= 10
    def changeState(self,s):
        if(self.state!=4):
            self.frame=0
            self.clock=0
            self.state=s
    def move(self,dX,dY):
        self.pos = (max(-15,min(width-(587*scale)+25,self.pos[0]+dX)),min(max(707*scale-30,self.pos[1]+dY),floor))
    def onAttack(self):
        global combo
        self.health -= 1*self.dr
        if(self.health <= 0):
            self.health = 0
            self.changeState(4)
        if(random.uniform(0,1) > self.combodef):combo = 0
    def heal(self,h):
        self.health += h
class Skeleton:
    def __init__(self):
        self.health = 1
        self.maxhealth = self.health
        self.str = 1
        self.diff = 0
        self.pos = (512,floor)
        self.scale = sScale
        self.clock = 0
        self.frame = 0
        self.speed = 1.5
        self.atkspeed = 24
        self.state = 0#0-idle 1-left 2-right 3-attack 4-ded
        self.offset = (-5,2)
        self.healthScale = 0.75#expo
        self.scoreScale = 1.1#expo
        self.speedScale = 0.02#linear
        self.moneyscale = 1.01
        #load idle animation
        self.hitbox = sIdle[0].get_rect(topleft=self.pos)
    def animate(self):
        (pW,pH)=self.pos
        pH -= 364*self.scale
        pH += self.offset[1]
        pW += self.offset[0]
        sW = 224*self.scale#surface width, useful
        if self.state == 0:
            self.hitbox = sIdle[self.frame].get_rect(topleft=(pW+10,pH))
            screen.blit(sIdle[self.frame],(pW,pH))
            if(self.clock%6 == 0):
                self.frame += 1
                self.frame %= 12
        elif self.state == 1:
            self.move(self.speed,0)
            self.hitbox = sRun[self.frame].get_rect(topleft=(pW+10,pH))
            screen.blit(pygame.transform.flip(sRun[self.frame],True,False),(pW,pH))
            if(self.clock%6 == 0):
                self.frame += 1
                self.frame %= 8
        elif self.state == 2:
            self.move(-1*self.speed,0)
            self.hitbox = sRun[self.frame].get_rect(topleft=(pW+10,pH))
            screen.blit(sRun[self.frame],(pW,pH))
            if(self.clock%6 == 0):
                self.frame += 1
                self.frame %= 8
        elif self.state == 3:
            self.hitbox = sAttack[self.frame].get_rect(topleft=(pW+10,pH))
            screen.blit(sAttack[self.frame],(pW,pH))
            if(self.clock%self.atkspeed == 0):
                self.frame += 1
                self.frame %= 8
        else:#death
            delay=25#frames from death to spawn
            if(self.frame <= 7):
                screen.blit(sDead[min(7,self.frame)],(pW,pH))
            elif(self.frame >= delay):
                screen.blit(sSpawn[min(max(self.frame-delay,0),9)],(pW,pH))
            else:
                self.pos = (random.randint(5,750),floor)
            if(self.clock%6==0):
                self.frame += 1
                if(self.frame>=delay+9):
                    self.changeState(0)
        if self.state!=4:#put other global stuffs here I guess
            #screen.blit(fontsmol.render(str(round(self.health,0)),True,(255,255,255)),(pW,pH-30))
            #goofy ahhh surfaces
            height=22
            surface = pygame.Surface((sW-30,height))
            surface.set_colorkey(white)
            #draw the health bar
            p=100*self.health/self.maxhealth#health percentage
            inPadding = 4#limits the thickness of the rectangle
            w=sW-30-(inPadding*2)#how long the line goes ah
            pygame.draw.rect(surface,white,(0,0,sW-30,height),width=0)
            pygame.draw.rect(surface,black,(0,0,sW-30,height),width=inPadding)
            if p>=75:
                pygame.draw.rect(surface,green,(inPadding,inPadding,w*(p/100),height-inPadding*2),width=0)
            elif p>=25:
                pygame.draw.rect(surface,yellow,(inPadding,inPadding,w*(p/100),height-inPadding*2),width=0)
            else:
                pygame.draw.rect(surface,red,(inPadding,inPadding,w*(p/100),height-inPadding*2),width=0)
            screen.blit(surface,(pW,pH-24))
            screen.blit(fontverysmol.render("{}".format(self.diff),True,white),(pW+w+inPadding*2+2,pH-24))
            
        self.clock += 1
    def changeState(self,s):
        self.clock = 0
        self.frame = 0
        self.state = s if (knight.state!=4) else 0
    def onAttack(self):
        self.health -= 1*knight.str
        if(random.random() > 1-(0.01*(self.diff**0.9))):self.frame = max(0,self.frame-1)
        if(swordLevel>=3 and random.random()<0.05):self.health -= 1.5*knight.str#crit
        global score,combo,killcount,scoretextscale
        if(scoretextscale>=1.2):scoretextscale=1.25
        else:scoretextscale+=0.02
        if(self.health<=0):
            score += 250*int(1+(self.diff/5)) + int(50*(combo**self.scoreScale)) + 10*killcount + 500*bglevel
            knight.money += int(7*((1+bglevel)**2) + random.uniform(0,combo**self.moneyscale) + random.uniform(0,self.diff**self.moneyscale))
            combo += 1
            self.diff = int(((combo**1.1)+1+15*bglevel+(score//25000))*random.uniform(0.9,1.1))
            self.clock = 0
            self.frame = 0
            self.state = 0
            scoretextscale += 0.1
            self.atkspeed = max(24-int(self.diff/10),3)#max speed : 210 difficulty
            self.health = 1+(self.diff**self.healthScale)
            self.maxhealth = self.health
            self.speed = 1.5+(self.diff*self.speedScale)
            self.hitbox = pygame.Rect(-1,-1,-1,-1)
            self.changeState(4)
            killcount += 1
            if killcount%30 == 0:knight.heal(1)
        else:
            score += 15 + int(5*(combo**1.05))
    def move(self,dX,dY):
        (x,y) = self.pos
        self.pos = (x+dX,y+dY)
knight = character()
#enemy = Skeleton()
enemies = []
enemies.append(Skeleton())
#enemies.append(Skeleton())
clock = pygame.time.Clock()
left_key = False
right_key = False
jump_key = False
while running:
    if(score >= 100000):
        bglevel = 2
        if len(enemies)==1:
            enemies.append(Skeleton())
            enemies[1].healthScale = 0.85
            enemies[1].scoreScale = 1.15
            enemies[1].speedScale = 0.05
            enemies[1].moneyscale += 0.01
        elif score >= 1000000 and len(enemies)==2:
            enemies.append(Skeleton())
            enemies[1].healthScale = 0.9
            enemies[1].scoreScale = 1.2
            enemies[1].speedScale = 0.075
            enemies[1].moneyscale += 0.02
    elif(score >= 10000):bglevel = 1
    screen.blit(background[bglevel],(0,0))
    #update position
    if(left_key and knight.state!=4):
        knight.move(-5,0)
    if(right_key and knight.state!=4):
        knight.move(5,0)
    if(jump_key and knight.state!=4):
        knight.move(0,-5)
    else:
        knight.move(0,5)
    #enemy update loop
    for enemy in enemies:
        if(enemy.state!=4 and knight.state!=4 and random.random()<0.9):
            if(knight.hitbox.colliderect(enemy.hitbox)):
                if(enemy.state!=3):
                    enemy.changeState(3)
                elif(enemy.frame==7):
                    knight.onAttack()
                    enemy.changeState(0)
            elif(knight.pos[0] > enemy.pos[0] and enemy.state!=1 and enemy.clock>=10):#move towards the player
                enemy.changeState(1)
            elif(knight.pos[0] < enemy.pos[0] and enemy.state!=2 and enemy.clock>=10):
                enemy.changeState(2)
            elif(knight.pos[0] == enemy.pos[0] and enemy.state!=0):
                enemy.changeState(0)
    #render
    for i in range(7):#rendering them as a grid, 7*4
        screen.blit(ground[1],(i*128,floor))
    if(knight.state == 4):
        screen.blit(gameover,(330,10))
    screen.blit(objects[1],(355,floor-93))
    screen.blit(objects[8],(712,floor-76))
    screen.blit(objects[6],(765,floor-64))
    screen.blit(objects[8],(865,floor-76))
    screen.blit(objects[7],(658,floor-55))
    screen.blit(objects[4],(-150,floor-239))
    screen.blit(heart,(padding,padding))
    screen.blit(coin,(padding,32+padding))
    screen.blit(sword[swordLevel],(padding+128,padding+32))
    screen.blit(armor[armorLevel],(padding+128,padding))
    screen.blit(skull[min(3,killcount//75)],(padding,64+padding))
    screen.blit(font.render(str(round(clock.get_fps(),1)),True,white),(0,480))
    screen.blit(knight.getLifeText(),(padding+32,padding))
    screen.blit(fontsmol.render("{}".format(knight.money),True,white),(padding+32,padding+32))
    screen.blit(font.render("{:.1f}".format(knight.str),True,white),(padding+180,padding+32))
    if swordLevel<5:screen.blit(fontsmol.render("({})".format(swordCost[swordLevel]),True,yellow),(padding+240,padding+32))
    if armorLevel<5:screen.blit(fontsmol.render("({})".format(armorCost[armorLevel]),True,yellow),(padding+240,padding))
    screen.blit(font.render("{:.1f}".format(knight.dr),True,white),(padding+180,padding))
    screen.blit(font.render("{}".format(killcount),True,white),(padding+32,padding+64))
    for enemy in enemies:
        enemy.animate()
    knight.renderScoreText()
    knight.animate()
    #gems
    gemCount=0
    if(swordLevel==5 and armorLevel==5):gemCount+=1
    if(score>=scoreR[2]):gemCount+=3
    elif(score>=scoreR[1]):gemCount+=2
    elif(score>=scoreR[0]):gemCount+=1
    if(killcount>=killR[2]):gemCount+=3
    elif(killcount>=killR[1]):gemCount+=2
    elif(killcount>=killR[0]):gemCount+=1
    if(knight.money>=moneyR[2]):gemCount+=3
    elif(knight.money>=moneyR[1]):gemCount+=2
    elif(knight.money>=moneyR[0]):gemCount+=1
    for i in range(gemCount):
        if i<=4:screen.blit(gems[i],(width-padding-(32*(5-i)-10),padding+75))#row 0
        else:screen.blit(gems[i],(width-padding-(32*(10-i)-10),padding+107))#row 1
    pygame.display.update()
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            running=False
            break
        #Keyboard
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_SPACE:
                jump_key=True
                knight.changeState(2)
                print("jump")
            elif event.key == pygame.K_a:
                left_key=True
                knight.changeState(1)
                print("left")
            elif event.key == pygame.K_d:
                right_key=True
                knight.changeState(1)
                print("right")
            elif event.key == pygame.K_f or event.key==pygame.K_j or event.key==pygame.K_g or event.key==pygame.K_h:
                knight.changeState(3)
                for enemy in enemies:
                    if knight.hitbox.colliderect(enemy.hitbox):enemy.onAttack()
            elif event.key == pygame.K_1 and swordLevel<5:
                if(knight.money >= swordCost[min(4,swordLevel)]):
                    swordLevel += 1
                    knight.str = swordStr[swordLevel]
                    knight.money -= swordCost[swordLevel-1]
            elif event.key == pygame.K_2 and armorLevel<5:
                if(knight.money >= armorCost[min(4,armorLevel)]):
                    armorLevel += 1
                    knight.health += 1
                    knight.maxhealth += 1
                    knight.dr -= 0.1
                    knight.combodef += 0.05
                    knight.money -= armorCost[armorLevel-1]
        if event.type == pygame.KEYUP:
            if event.key == pygame.K_SPACE:
                knight.changeState(0)
                jump_key=False
            elif event.key == pygame.K_a:
                knight.changeState(0)
                left_key=False
            elif event.key == pygame.K_d:
                knight.changeState(0)
                right_key=False
            elif event.key == pygame.K_f or event.key==pygame.K_j or event.key==pygame.K_g or event.key==pygame.K_h:
                knight.changeState(0)
    clock.tick(60)
