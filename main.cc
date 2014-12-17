#include "main.h"

// Computes the total density on a node
double density(VelocitySet &set, Node node)
{
    double density = 0;
    for (size_t i = 0; i < set.nDimensions; ++i)
        density += node.distributions[i].value;

    return density;
}

// Coputes the projected velocity in each dimension
double *velocity(VelocitySet &set, Node node)
{
    size_t nDirections = set.nDirections;
    size_t nDimensions = set.nDimensions;
    double nodeDensity = density(set, node);
    // bleh, lelijk
    double *velocity = new double[nDimensions];
    for (size_t dim = 0; dim < nDimensions; ++dim)
        velocity[dim] = 0;

    // compute the velocity in each dimension taking in account
    // the form of our velocity set
    for (size_t dir = 0; dir < nDirections; ++dir)
        for (size_t dim = 0; dim < nDimensions; ++dim)
            velocity[dim] += node.distributions[dir].value
                 * set.weights[dir] * set.directions[dir][dim];

    for (size_t dim = 0; dim < nDimensions; ++dim)
        velocity[dim] /= nodeDensity;

    return velocity;
}

double *equilibrium(VelocitySet &set, Node node)
{
    size_t nDirections = set.nDirections;
    size_t nDimensions = set.nDimensions;
    double speedOfSoundSquared = set.speedOfSoundSquared;
    double node_density = density(set, node);
    double *node_velocity = velocity(set, node);
    // double *distributions = node.distributions;

    // Pre calculate the speed of the node
    double speedSquared = 0;
    for (size_t dim = 0; dim < nDimensions; ++dim)
        speedSquared += node_velocity[dim] * node_velocity[dim];
    speedSquared /= (2 * speedOfSoundSquared);

    double *equilibrium = new double[nDimensions];
    for (size_t dir = 0; dir < nDirections; ++dir)
    {
        double cu = 0;
        for (size_t dim = 0; dim < nDimensions; ++dim)
            cu = set.directions[dir][dim] * node_velocity[dir];
        cu /= speedOfSoundSquared;

        equilibrium[dir] = node_density * set.weights[dir] * (
            1 +
            cu +
            cu * cu / 2 -
            speedSquared
        );
    }

    return equilibrium;
}

bool isOnBoundary(size_t x, size_t y, size_t z)
{
    return (x == 0 && y == 0);
}

void report(VelocitySet &set, Node *nodes, size_t totalNodes)
{
    std::cout << "Did some iterations" << '\n';

    double total_density = 0;

    for (size_t idx = 0; idx < totalNodes; ++idx)
        total_density += density(set, nodes[idx]);

    std::cout << "Total density: " << total_density << '\n';

    size_t nDimensions = set.nDimensions;
    double *total_velocity = new double[nDimensions];
    for (size_t dim = 0; dim < nDimensions; ++dim)
        total_velocity[dim] = 0;

    for (size_t idx = 0; idx < totalNodes; ++idx)
    {
        double *node_velocity = velocity(set, nodes[idx]);
        for (size_t dim = 0; dim < nDimensions; ++dim)
            total_velocity[dim] += node_velocity[dim];

        delete[] node_velocity;
    }


    std::cout << "Total velocity: ";
    for (size_t dim = 0; dim < nDimensions; ++dim)
        std::cout << total_velocity[dim] << '\t';
    std::cout << '\n';

    delete[] total_velocity;
}

void collideNode(VelocitySet &set, Node &node)
{
    double * node_equilibrium = equilibrium(set, node);

}

void collision(VelocitySet &set, Node *nodes, size_t totalNodes)
{
    // for (size_t idx = 0; idx < totalNodes; ++idx)
    // {
    //     var equilibrium = this.getEquilibrium();
    //         var velocitySet = Config.get('velocity-set');
    //         // var force = 10;
    //         for (var k = 0; k < this.distributions.length; k++) {
    //                 this.distributions[k] = this.distributions[k] -
    //                     (this.distributions[k] - equilibrium[k]) / relaxationTime;
    //                      // + 3 * velocitySet[k].dy * velocitySet[k].w * force;

    //             if (this.distributions[k] < 0) {
    //                 // console.log("Distribution is negative!", this.distributions[k]);
    //             }
    //         };
    // }
}

void stream(VelocitySet &set, Node *nodes, size_t totalNodes)
{

}

/*size_t neighbourIdxForDistribution(size_t nodeIdx, size_t distributionIdx, VelocitySet &set)
{
    // if isnt a boundary
    // get the 2D idx x, y for nodeIdx,
    // add set.directions[distributionIdx][0] to x and set.directions[distributionIdx][1] to y
    // return a transformation from 2d idx to 1d idx

    // if is a periodic boundary
    // do some modulo things

    // if other boundary, do other things, maybe this should be replaced with a set neighbours method?
}*/

void initializeNodeAt(VelocitySet &set, Node *nodes, size_t x, size_t y, size_t dx, size_t dy)
{
    if (set.nDimensions != 2)
        throw "Dimensie van velocity set is niet 2";
    size_t idx = x * dx + y;
    size_t nDirections = set.nDirections;
    nodes[idx].type = Cell;

    Distribution *distributions = new Distribution[nDirections];
    for (size_t dir = 0; dir < nDirections; ++dir)
    {
        distributions[dir].value = set.weights[dir];
        // periodic boundary
        size_t neighbour_x = x + set.directions[dir][0];
        size_t neighbour_y = y + set.directions[dir][1];
        size_t neighbour_idx = (neighbour_x % dx) * dx + (neighbour_y % dy);
        distributions[dir].neighbour = &nodes[neighbour_idx].distributions[dir].value;
    }
    nodes[idx].distributions = distributions;
}

Node *initialize(VelocitySet &set, size_t &totalNodes)
{
    // Initialize the velocity set (note: should be available to all processors)
    initializeVelocitySet(set);

    size_t dx = 10;
    size_t dy = 10;

    totalNodes = dx * dy;
    Node *nodes = new Node[totalNodes];

    for (size_t x = 0; x < dx; ++x)
        for (size_t y = 0; y < dy; ++y)
            initializeNodeAt(set, nodes, x, y, dx, dy);

    return nodes;
}

int main(int argc, char **argv)
{
    VelocitySet set;
    Node *nodes;
    size_t totalNodes = 0;
    nodes = initialize(set, totalNodes);

    size_t iterations = 1000;
    for (size_t iter = 0; iter < iterations; ++iter)
    {
        collision(set, nodes, totalNodes);
        stream(set, nodes, totalNodes);

        if (iter % 100 == 0)
            report(set, nodes, totalNodes);
    }

    // Free up the memory taken by our velocity set
    size_t dimensions = set.nDimensions;
    delete[] set.weights;
    for (size_t i = 0; i < dimensions; ++i)
        delete[] set.directions[i];
    delete[] set.directions;

    for (size_t i = 0; i < totalNodes; ++i)
        delete[] nodes[i].distributions;
    delete[] nodes;

    return 0;
}
