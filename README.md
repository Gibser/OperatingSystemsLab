# Operating Systems Lab Project

### Description
Design and development of a multiplayer game written in C for UNINA Computer Science's course "Operating Systems Laboratory"

### Game Description

#### Goal
The goal of the game is to collect items scattered around the map and place them in the appropriate storerooms within the time limit. 
The player with the most items delivered wins.

#### World Map
The game map is represented by a matrix within which are the following elements:
- Players (represented as a letter)
- Warehouse (represented as a number)
- Items
- Obstacles

Obstacles on the map are invisible to all players. 
Each player has his or her own customized view of the map, and when he or she encounters an obstacle that does not allow movement,
it is marked on the map with an "x" visible only to that player.
For each new game, the map is randomly generated, with a variable size. The number of items and warehouses and the arrangement of obstacles is also done pseudo-randomly.


<p align="center">
  <img src="https://github-production-user-asset-6210df.s3.amazonaws.com/58134090/270114098-a9222608-def2-4618-8997-51069e457442.png">
</p>




### Authors
[cappadavide](https://github.com/cappadavide)

[Gibser](https://github.com/Gibser)
