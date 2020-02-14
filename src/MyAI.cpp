// ======================================================================
// FILE:        MyAI.cpp
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

#include "MyAI.hpp"

MyAI::MyAI()
    : Agent()
{
}
	
Agent::Action MyAI::getAction (bool stench, bool breeze, bool glitter, bool bump, bool scream)
{
    if (bump)
    {
        updatePosition(true);
        markWalls();
    }
    updateMap(stench, breeze);
    if (scream)
    {
        wumpusAlive = false;
        this->state = AgentState::Exploring;
    }
    if (glitter && this->state != AgentState::Returning)
    {
        clearActionQueue();
        this->hasGold = true;
        this->state = AgentState::Returning;
        return Agent::Action::GRAB;
    }
    if (breeze && this->state != AgentState::Returning)
    {
        clearActionQueue();
        this->state = AgentState::Returning;
    }
    else if (stench && wumpusAlive && this->state != AgentState::Returning) 
    {
        if (this->hasArrow)
        {
            hasArrow = false;
            return Agent::Action::SHOOT;
        }
        this->state = AgentState::Returning;
    }
    if (this->state == AgentState::Returning && !hasGold && !breeze && (!stench || !wumpusAlive) && !possibleDirections().empty())
    {
         this->state = AgentState::Exploring;
    }
    if (actionQueue.empty())
        takeAction();
    return returnAction();
}

Agent::Action MyAI::returnAction()
{
    Agent::Action action = actionQueue.front();
    actionQueue.pop();
    if (action == Agent::Action::FORWARD)
        updatePosition();
    else if (action == Agent::Action::TURN_LEFT || action == Agent::Action::TURN_RIGHT)
        updateDirection(action);
    return action;
}

std::vector<std::pair<int, int>> MyAI::getStenchTiles()
{
    std::vector<std::pair<int, int>> stenchTiles;
    for (int x = 0; x < 7; ++x)
        for (int y = 0; y < 7; ++y)
            if (this->mapAt({x, y}).stench)
                stenchTiles.push_back({x, y});
    return stenchTiles;
}

void MyAI::locateWumpus()
{
    std::vector<std::pair<int, int>> stenchTiles = getStenchTiles();
    std::swap(stenchTiles[0].first, stenchTiles[1].first);
    (mapAt(stenchTiles[0]).visited) ? mapAt(stenchTiles[1]).wumpus = true : mapAt(stenchTiles[0]).wumpus = true;
}

void MyAI::updateDirection(Agent::Action action)
{
    switch (this->facing)
    {
        case Direction::Up:
            (action == Agent::Action::TURN_LEFT) ? this->facing = Direction::Left : this->facing = Direction::Right;
            break;
        case Direction::Down:
            (action == Agent::Action::TURN_LEFT) ? this->facing = Direction::Right : this->facing = Direction::Left;
            break;
        case Direction::Left:
            (action == Agent::Action::TURN_LEFT) ? this->facing = Direction::Down : this->facing = Direction::Up;
            break;
        case Direction::Right:
            (action == Agent::Action::TURN_LEFT) ? this->facing = Direction::Up : this->facing = Direction::Down;
            break;
    }
}

void MyAI::takeAction()
{
    std::vector<Direction> directions;
start:
    switch (this->state)
    {
        case AgentState::Exploring:
            directions = possibleDirections();
            if (directions.empty())
            {
                this->state = AgentState::Returning;
                goto start;
            }
            for (auto direction : directions)
                if (direction == this->facing)
                {
                    move(direction);
                    return;
                }
            move(directions[0]);
            break;
        case AgentState::Hunting:
            break;
        case AgentState::Returning:
            if (this->position == std::pair<int, int>{0, 0})
                this->actionQueue.front() = Agent::Action::CLIMB;
            else
                move(shortestPath());
            break;
    }
}

void MyAI::updatePosition(bool hitWall)
{
    switch(this->facing)
    {
        case Direction::Up:
            (hitWall) ? this->position.second -= 1 : this->position.second += 1;
            break;
        case Direction::Down:
            (hitWall) ? this->position.second += 1 : this->position.second -= 1;
            break;
        case Direction::Left:
            (hitWall) ? this->position.first += 1 : this->position.first -= 1;
            break;
        case Direction::Right:
            (hitWall) ? this->position.first -= 1 : this->position.first += 1;
            break;
    }
}

void MyAI::markWalls()
{
    for (int i = 0; i < 7; ++i)
    {
        if (this->facing == Direction::Up)
            mapAt(std::pair<int, int>(i, this->position.second + 1)).wall = true;
        else if (this->facing == Direction::Right)
            mapAt(std::pair<int, int>(this->position.first + 1, i)).wall = true;
    }
}

void MyAI::updateMap(bool stench, bool breeze)
{
    mapAt(this->position) = {true, false, breeze, stench, false};
}

MyAI::Tile& MyAI::mapAt(std::pair<int, int> coordinate)
{
    return map[6 - coordinate.second][coordinate.first];
}

void MyAI::move(MyAI::Direction direction)
{
    std::vector<Agent::Action> actions = this->rotationGrid[this->facing][direction];
    for (auto it = actions.begin(); it != actions.end(); ++it)
        this->actionQueue.push(*it);
    this->actionQueue.push(Agent::Action::FORWARD);
}

MyAI::Direction MyAI::relationalDirection(std::pair<int, int> targetTile)
{
    std::pair<int, int> relation = std::pair<int, int>{this->position.first - targetTile.first, this->position.second - targetTile.second};
    if(relation == std::make_pair(0, 1))
        return Direction::Down;
    else if(relation == std::make_pair(0, -1))
        return Direction::Up;
    else if(relation == std::make_pair(1, 0))
        return Direction::Left;
    else if(relation == std::make_pair(-1, 0))
        return Direction::Right;
    else
        throw MyAI::NonAdjacentTileException{};
}

bool MyAI::inferSafeTile(std::pair<int,int> coordinate)
{
    auto up = std::make_pair(coordinate.first, coordinate.second + 1);
    auto down = std::make_pair(coordinate.first, coordinate.second - 1);
    auto left = std::make_pair(coordinate.first - 1, coordinate.second);
    auto right = std::make_pair(coordinate.first + 1, coordinate.second);
    if (inBounds(up) && inBounds(down))
    {
        auto upTile = mapAt(up);
        auto downTile = mapAt(down);
        if (upTile.visited && downTile.visited)
            if (!(upTile.breeze && downTile.breeze))
                if (!wumpusAlive || !(upTile.stench && downTile.stench))
                    return true;
    }
    if (inBounds(left) && inBounds(right))
    {
        auto leftTile = mapAt(left);
        auto rightTile = mapAt(right);
        if (leftTile.visited && rightTile.visited)
            if (!(leftTile.breeze && rightTile.breeze))
                if (!wumpusAlive || !(leftTile.stench && rightTile.stench))
                    return true;
    }
    return false;
}

bool MyAI::inBounds(std::pair<int,int> coordinate)
{
    if (coordinate.first >= 0 && coordinate.first < 7 && coordinate.second >= 0 && coordinate.second < 7)
        return true;
    return false;
}

bool MyAI::validCell(std::pair<int, int> coordinate)
{
    if (inBounds(coordinate))
    {
        Tile t = this->mapAt(coordinate);
        if (t.wall == false && t.wumpus == false && t.visited == false)
            return true;
    }
    return false;
}

bool MyAI::validReturnCell(std::pair<int, int> coordinate)
{
    if (inBounds(coordinate))
    {
        Tile t = this->mapAt(coordinate);
        if (t.visited == true)
            return true;
        else if (inferSafeTile(coordinate))
            return true;
    }
    return false;
}

std::pair<int, int> MyAI::applyDirection(std::pair<int, int> current, Direction d)
{
    switch (d)
    {
        case Direction::Up:
            return std::make_pair(current.first, current.second + 1);
        case Direction::Down:
            return std::make_pair(current.first, current.second - 1);
        case Direction::Left:
            return std::make_pair(current.first - 1, current.second);
        case Direction::Right:
            return std::make_pair(current.first + 1, current.second);
    }
}

std::vector<MyAI::Direction> MyAI::validMoves(std::pair<int, int> currentTile, const std::unordered_set<std::pair<int, int>>& pathTraveled)
{
    std::vector<MyAI::Direction> directions;
    if (pathTraveled.find(applyDirection(currentTile, Direction::Up)) == pathTraveled.end() && validReturnCell(applyDirection(currentTile, Direction::Up)))
        directions.push_back(Direction::Up);
    if (pathTraveled.find(applyDirection(currentTile, Direction::Down)) == pathTraveled.end() && validReturnCell(applyDirection(currentTile, Direction::Down)))
        directions.push_back(Direction::Down);
    if (pathTraveled.find(applyDirection(currentTile, Direction::Left)) == pathTraveled.end() && validReturnCell(applyDirection(currentTile, Direction::Left)))
        directions.push_back(Direction::Left);
    if (pathTraveled.find(applyDirection(currentTile, Direction::Right)) == pathTraveled.end() && validReturnCell(applyDirection(currentTile, Direction::Right)))
        directions.push_back(Direction::Right);
    return directions;
}

int MyAI::pathCost(std::unordered_set<std::pair<int, int>>& pathTraveled, std::pair<int, int> currentTile, Direction currentFacing)
{
    if (currentTile == std::make_pair(0, 0))
        return 0;
    else
    {
        pathTraveled.insert(currentTile);
        std::vector<Direction> moves = validMoves(currentTile, pathTraveled);
        if (moves.empty())
        {
            pathTraveled.erase(currentTile);
            return 100000;
        }
        std::vector<int> costs;
        for (auto move : moves)
            costs.push_back(rotationGrid[currentFacing][move].size() + 1 + pathCost(pathTraveled, applyDirection(currentTile, move), move));
        pathTraveled.erase(currentTile);
        return *(std::min_element(costs.begin(), costs.end()));
    }
}

MyAI::Direction MyAI::shortestPath()
{
    Direction result;
    int cheapest = std::numeric_limits<int>::max();
    std::unordered_set<std::pair<int, int>> pathTraveled;
    std::vector<Direction> moves = validMoves(this->position, pathTraveled);
    pathTraveled.insert(position);
    for (auto d : moves)
    {
        int pathcost = pathCost(pathTraveled, applyDirection(this->position, d), d) + rotationGrid[this->facing][d].size() + 1;
        if (pathcost < cheapest)
        {
            cheapest = pathcost;
            result = d;
        }
    }
    return result;
}

std::vector<MyAI::Direction> MyAI::possibleDirections()
{
    std::vector<MyAI::Direction> directions;
    if (validCell(std::make_pair(position.first + 1, position.second)))
        directions.push_back(Direction::Right);
    if (validCell(std::make_pair(position.first, position.second + 1)))
        directions.push_back(Direction::Up);
    if (validCell(std::make_pair(position.first - 1, position.second)))
        directions.push_back(Direction::Left);
    if (validCell(std::make_pair(position.first, position.second - 1)))
        directions.push_back(Direction::Down);
    return directions;
}

void MyAI::clearActionQueue()
{
    while (!actionQueue.empty())
        actionQueue.pop();
}
