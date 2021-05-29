# BulletBounce
Simple Bullet Hell Game developed in the Vienna Vulkan Engine as part of the Gaming Technologies course at the University of Vienna. 
This game uses a simple collision detection and response algorithm implemtned from scratch using the GJK algorithm. 
The main goal of this simple game was to incooperate everything learned furing the semester into on coherent project. 
## What is it? 
The goal of the game is to survive as many enemy waves as possible. The player controls a cube followed by three samller cubes representing the remaining lives. 
Three types of enemies were implemented. Static Tower enemies shooting in the players direction, enemies that try to rush down the player and deal damage on contact 
and enemies that run away when the player enteres there interaction area. 

To make the game a bit more interesting, only bullets that bounced at least once do damage to enemies. The cube not only be moved areound but also rotated seperately. 
This combination of motion gives a great range of mobility to the player. Only three player shots can be active at once and they will disapear after a set amount of bounces. 
To stay on top of the competion the player can try to hit projectiles traveling around the map to create more smaller projectiles. 
## Current State of the Project
I tried to compile the project with a newer version of the engine and a newer version of Visual Studio and it currently won't compile. The main problem seems to be with 
GLM function decleartions. When I have some free time during the next weeks I will try to get it running again. 

## Credits 
For providing the Vienna Vulkan Engine: https://github.com/hlavacs/ViennaVulkanEngine
