// ======================================================================
// FILE:        MyAI.hpp
//
// AUTHOR:      Abdullah Younis
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================

#ifndef MYAI_LOCK
#define MYAI_LOCK

#include "Agent.hpp"
#include <queue>
#include <stack>
#include <limits>
#include <vector>
#include <unordered_set>
#include <utility>
#include <algorithm>

// a simple hashing fuction to allow the use of std::pair as the key of STL containers
namespace std
{
    template<> struct hash<std::pair<int, int>>
    {
        inline size_t operator()(const std::pair<int, int> &v) const
        {
            std::hash<int> int_hasher;
            return int_hasher(v.first) ^ int_hasher(v.second);
        }
    };
}

class MyAI : public Agent
{
public:

    // Tile refers to each cell or room within the cave.
    // When the agent visits a tile, we mark it as having been visited, as well as copy the stench and breeze data.
    // If we are able to triangulate the position of the wumpus, we can mark wumpus as true to indicate the presence of the
    // wumpus at the tile.
    struct Tile
    {
        Tile ()
        {
            bool visited = false;
            bool wumpus = false;
            bool breeze = false;
            bool stench = false;
            bool wall = false;
        }

        Tile (bool v, bool w, bool b, bool s, bool wl)
        {
            visited = v;
            wumpus = w;
            breeze = b;
            stench = s;
            wall = wl;
        }

        bool visited;
        bool wumpus;
        bool breeze;
        bool stench;
        bool wall;
    };

    // Direction represents the direction that the agent is facing.
    enum Direction
    {
        Up,
        Down,
        Left,
        Right
    };

    // AgentState represents which state or mode the agent is in. Different states require different actions
    // to be made by the AI.
    enum AgentState
    {
        Exploring,
        Hunting,
        Returning
    };

    class NonAdjacentTileException{};

	MyAI(void);
	
	Action getAction(bool stench, bool breeze, bool glitter, bool bump, bool scream);

private:
    // move() pushes the required Agent::Actions to move in the given direction onto the actionQueue
    void move(Direction direction);

    // possibleDirections() returns a list of possible directions
    std::vector<Direction> possibleDirections();


    // markWalls() is called when the agent perceives a bump and marks either the top
    // or right set of walls of the cave depending upon the direction the agent is facing.
    void markWalls();

    // applyDirection() returns the coordinates of the tile resulting from applying the direction
    // to the current tile.
    std::pair<int, int> applyDirection(std::pair<int, int> current, Direction d);

    // validMoves() is a helper function that enumerates the directions the Agent could take at a tile along the current return
    // path traveled. validMoves() will never allow the agent to move in the direction of a tile it has already visited along the
    // current path.
    std::vector<Direction> validMoves(std::pair<int, int> currentTile, const std::unordered_set<std::pair<int, int>>& pathTraveled);

    // validReturnCell() checks if the tile at the coordinate is considered a valid cell to travel on for the return path.
    // A cell is a valid return cell if:
    // 1) it has been visited before, OR
    // 2) we can infer the tile's safety
    bool validReturnCell(std::pair<int, int> coordinate);

    // pathCost() performs a depth first search on the map to determine the total cost of a path. It returns the cost of the
    // cheapest path.
    int pathCost(std::unordered_set<std::pair<int, int>>& pathTraveled, std::pair<int, int> currentTile, Direction currentFacing);

    // shortestPath() returns the direction the Agent should travel in order to reach the exit taking the shortest path.
    MyAI::Direction shortestPath();

    // inferSafeTile() attempts to infer whether or not a tile is safe to travel on despite having never visited it before.
    // This is useful in the shortest path first function to take shortcuts home.
    bool inferSafeTile(std::pair<int, int> coordinate);

    // inBounds() is a helper function that determines whether or not a coordinate is within the bounds of the
    // maximum map size. This function does not check whether or not the coordinate is within the walls of the
    // map.
    bool inBounds(std::pair<int, int> coordinate);

    // takeAction() determines which Actions or set of Actions to push onto the actionQueue
    // depending upon the current state of the agent.
    // If the agent state is exploring, a random unexplored cell will be chosen and the required actions
    // to move the agent to that cell will be pushed onto the actionQueue.
    // If the agent state is hunting, a predefined sequence of actions will be pushed onto the actionQueue
    // in an attempt to locate or kill the wumpus.
    // If the agent state is returning, the agent will try to exit the cave.
    void takeAction();

    // updatePosition() keeps track of the agent's position data as it moves through the cave.
    // When hitWall is false, we assume the agent has successfully moved into the next cell, so we
    // can update the position data accordingly.
    // If we hit a wall, calling the function with hitWall = true will put our position data back to the cell
    // we tried to move from since our agent was unable to move cells.
    void updatePosition(bool hitWall = false);

    // updateDirection() takes either Agent::Action::TURN_RIGHT or Agent::Action::TURN_LEFT as an argument,
    // and updates the agent's direction facing accordingly. Calling this function with an action outside of
    // rotation actions results in undefined behavior.
    // TODO add validation for the argument
    void updateDirection(Agent::Action action);

    // updateMap() takes the stench and breeze of the current room as arguments and marks them on the map.
    void updateMap(bool stench, bool breeze);

    // locateWumpus() is a subroutine that is called at the end of the AgentState::Hunting state, and will
    // triangulate the position of the wumpus in the cave and mark the map accordingly.
    void locateWumpus();

    // relationalDirection() takes the coordinates of a tile in the form <x, y>.
    // The tile must be directly adjacent to the agent's current position.
    // Returns the direction needed to travel in order for the agent to move to the target
    // tile from its current position.
    // Throws MyAI::NonAdjacentTileException if targetTile is not an adjacent tile.
    MyAI::Direction relationalDirection(std::pair<int, int> targetTile);

    // returnAction() manages the returning of the front member of the actionQueue.
    // This function calls updatePosition() and updateDirection() accordingly, and also
    // manages which actions are pushed onto the previousAction stack.
    Agent::Action returnAction();

    // getStenchTiles() is a helper function for locateWumpus() that returns a pair of pairs
    // representing the coordinates of the two tiles of diagonal stenches.
    std::vector<std::pair<int, int>> getStenchTiles();

    // mapAt() allows access to the Tile data that we have mapped. The argument is in the form
    // <x , y> with x and y referring to player coordinates. The function translates the player coordinates
    // to the proper 2D array indeces.
    MyAI::Tile& mapAt(std::pair<int, int> coordinate);

    // validCell() is a helper function for possibleDirections() and returns true if the
    // cell at the given coordinates (from the player's perspective) is a valid cell; false otherwise.
    // The arguments are in the form <x , y> with x referring to horizontal movement and y referring to vertical movement.
    // The agent starts at <0 , 0>.
    bool validCell(std::pair<int, int> coordinate);

    // clearActionQueue() clears all queued actions from the action queue. This function is typically called
    // when the agent enters 'Returning' state.
    void clearActionQueue();

    // actionQueue provides a method for complex actions to be strung together by the AI and executed
    // in sequence without interruption
    std::queue<Agent::Action> actionQueue;

    // previousPosition is a stack of std::pair<int, int> coordinates in the form <x , y> where each
    // coordinate represents a tile that the agent has been. The agent starts at tile <0, 0> so previousPosition
    // is initialized with <0, 0> on the top of the stack. When the agent returns an Action::FORWARD, the coordinate
    // for the new room the agent will enter is pushed onto the stack. In the event of the agent hitting a wall,
    // the top of stack will be popped as the agent was unable to move rooms.
    std::stack<std::pair<int, int>> previousPosition;

    // position keeps track of the agent's current position in the cave.
    // It is in the form <x , y> with x referring to horizontal movement and y referring to
    // vertical movement. The agent will always start at <0, 0>. Moving the agent right
    // will result in x coordinate increasing; moving the agent up will result in the
    // y coordinate increasing.
    std::pair<int, int> position = std::pair<int, int>(0, 0);

    // facing describes which direction the agent is currently facing.
    Direction facing = Direction::Right;

    // state is the current state of the Agent. The options are Exploring, Hunting, and Returning.
    // Exploring state is the default state of the AI agent, in which the AI will choose a random direction to move.
    // Hunting state is activated the first time the agent perceives a stench, and ends after finding a second stench.
    // Returning state is activated when either the gold is recovered or a breeze is perceived, and the agent will return home immediately.
    AgentState state = AgentState::Exploring;

    MyAI::Tile map[7][7];

    bool hasArrow = true;

    bool hasGold = false;

    bool wumpusAlive = true;

    // rotationGrid provides a static lookup table to determine the proper number of rotations required to
    // face a particular direction from the current direction.
    // Accessing the array should be done in the format [currentDirection][desiredDirection] and can be
    // done using the Direction enum implementation.
    const std::vector<Agent::Action> rotationGrid[4][4] = 
    {
    {{}, {Agent::Action::TURN_LEFT, Agent::Action::TURN_LEFT}, {Agent::Action::TURN_LEFT}, {Agent::Action::TURN_RIGHT}},
    {{Agent::Action::TURN_LEFT, Agent::Action::TURN_LEFT}, {}, {Agent::Action::TURN_RIGHT}, {Agent::Action::TURN_LEFT}},
    {{Agent::Action::TURN_RIGHT}, {Agent::Action::TURN_LEFT}, {}, {Agent::Action::TURN_LEFT, Agent::Action::TURN_LEFT}},
    {{Agent::Action::TURN_LEFT}, {Agent::Action::TURN_RIGHT}, {Agent::Action::TURN_LEFT, Agent::Action::TURN_LEFT}, {}},
    };
};

#endif
