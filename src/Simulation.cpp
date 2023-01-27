#include "Simulation.h"
#include <span>
#include <iostream>

Simulation::Simulation() :
    MassPoints({}) {}

void Simulation::render(int WindowLength, int FPS, float speed)
{
    window_length = WindowLength;
    frames_per_second = FPS;
    delta_time = speed / (float)FPS;

    SCALE = (float)window_length / 1000.0f;

    sf::RenderWindow window(sf::VideoMode(window_length, window_length), "Oscar!! TOUCH MY BALLS???", sf::Style::Titlebar);
    window.setFramerateLimit(FPS);
    sf::Event event;

    const float COOLDOWN = 0.005; //seconds
    float timePassed = COOLDOWN + 1;
    MassPoint CurrMassPointData;
    PressureSpringMassModel currentModelData;
    sf::Vector2i MousePosition;

    std::cout << window.getSize().x;
    bool leftclickpressed = true;

    Vector2 mouse;

    while (window.isOpen())
    {
        mouse = toSimulationSpace(
            Vector2(
            sf::Mouse::getPosition(window).x, 
            sf::Mouse::getPosition(window).y));

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::R)
                {
                    std::cout << "Clearing Everything..." << std::endl;
                    MassPoints.clear();
                    Springs.clear();
                    SpringMassModels.clear();
                    
                }
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            if (timePassed > COOLDOWN)
            {
                timePassed = 0;
                //std::cout << "Mouse Pressed";
                CurrMassPointData = MassPoint();
                CurrMassPointData.collideRadius = 10;
                CurrMassPointData.mass = 1;
                CurrMassPointData.velocity = Vector2(-50, 10);
                MousePosition = sf::Mouse::getPosition(window);
                CurrMassPointData.position = toSimulationSpace(Vector2(MousePosition.x, MousePosition.y));//*0 + Vector2(0,0);
                MassPoints.push_back(CurrMassPointData);
                if (!leftclickpressed)
                {
                    leftclickpressed = true;
                    currentModelData = PressureSpringMassModel();
                    currentModelData.nRT = 20000;
                }
                currentModelData.MassPoints.push_back(MassPoints.size() - 1);
            }
        }
        else if (leftclickpressed)
        {
            generateSprings(currentModelData);
            SpringMassModels.push_back(currentModelData);
            leftclickpressed = false;
        }


        window.clear(); drawMassPoints(&window);
        applyMassPointPhysics(delta_time, !leftclickpressed, mouse);
        applySprings(delta_time, &window); 
        applyPressureMassPhysiscs(delta_time);
        window.display();

        timePassed += delta_time;
    }
}


Vector2 Simulation::toScreenSpace(Vector2 point)
{
    return (point.flipY() + Vector2(500.0f, 500.0f)) * SCALE;
}

Vector2 Simulation::toSimulationSpace(Vector2 point)
{
    return ((point / SCALE) - Vector2(500.0f, 500.0f)).flipY();
}

void Simulation::drawMassPoints(sf::RenderWindow* pWindow)
{
    for (MassPoint& mass_point : MassPoints)
    {
        sf::CircleShape MassPointCollisionLayer(mass_point.collideRadius * SCALE);
        sf::CircleShape MassPointRepresentation(2);


        Vector2 ScreenCoordinates = toScreenSpace(mass_point.position);
        MassPointCollisionLayer.setPosition(
            ScreenCoordinates.x - (mass_point.collideRadius * SCALE),
            ScreenCoordinates.y - (mass_point.collideRadius * SCALE));
        MassPointRepresentation.setPosition(
            ScreenCoordinates.x - (2 * SCALE),
            ScreenCoordinates.y - (2 * SCALE));
        MassPointCollisionLayer.setFillColor(sf::Color(0, 0, 0, 0));
        if (!mass_point.iscollide) {
            MassPointCollisionLayer.setOutlineColor(sf::Color(255, 255, 255, 255));
        }
        else
        {
            mass_point.iscollide = false;
            MassPointCollisionLayer.setOutlineColor(sf::Color(255, 0, 0, 255));
        }
        MassPointCollisionLayer.setOutlineThickness(1);
        MassPointRepresentation.setFillColor(sf::Color(255, 255, 255, 255));

        //std::cout << "x : " << MassPointRepresentation.getPosition().x << ", y : " << MassPointRepresentation.getPosition().y << std::endl;

       

        pWindow->draw(MassPointCollisionLayer);
        pWindow->draw(MassPointRepresentation);
    }
}

void Simulation::applySprings(float dt, sf::RenderWindow* pWindow)
{
    Vector2 ScreenCoordsA;
    Vector2 ScreenCoordsB;
    for (Spring& spring : Springs)
    {

        spring.applyForces(MassPoints, dt);

        ScreenCoordsA = toScreenSpace(MassPoints[spring.A].position);
        ScreenCoordsB = toScreenSpace(MassPoints[spring.B].position);
        //std::cout << ScreenCoordsA.x << std::endl;
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(ScreenCoordsA.x, ScreenCoordsA.y)),
            sf::Vertex(sf::Vector2f(ScreenCoordsB.x, ScreenCoordsB.y))
        };


        pWindow->draw(line, 2, sf::Lines);
    }
}

void Simulation::applyMassPointCollision(MassPoint* mass_point)
{
    if (abs(mass_point->position.x) + mass_point->collideRadius >= 500)
    {
        if (mass_point->position.x > 0)
        {
            mass_point->position.x = 500 - mass_point->collideRadius;
        }
        else if (mass_point->position.x < 0)
        {
            mass_point->position.x = -500 + mass_point->collideRadius;
        }
        mass_point->velocity.flipX();
    }
    if (abs(mass_point->position.y) + mass_point->collideRadius >= 500)
    {
        if (mass_point->position.y > 0)
        {
            mass_point->position.y = 500 - mass_point->collideRadius;
        }
        else if (mass_point->position.y < 0)
        {
            mass_point->position.y = -500 + mass_point->collideRadius * 2;
        }
        mass_point->velocity.flipY();
    }

}

void Simulation::applyMassPointPhysics(float dt, bool push, Vector2 mouse)
{
    for (MassPoint& mp : MassPoints)
    {
        mp.iscollide = false;
    }
    for (int iteration = 0.0f; MassPoint& mass_point : MassPoints)
    {
        mass_point.velocity += Vector2(0.0f, -2000.0f) * dt / mass_point.mass;
        mass_point.position += mass_point.velocity * dt;

        if ((mouse - mass_point.position).length() < 60.0f && push)
        {
            mass_point.collide(mouse, Vector2(), 60.0f);
        }

        applyMassPointCollision(&mass_point);
        for (MassPoint& inner_mass_point : 
            std::span(MassPoints.begin() + iteration + 1, MassPoints.end())) 
        {
            bool collision = mass_point.isColliding(inner_mass_point);
            if (collision == true)
            {
                //std::cout << collision << std::endl;
                mass_point.collide(inner_mass_point);
                //mass_point.velocity = Vector2();
                //inner_mass_point.velocity = Vector2();
            }
        }
        iteration++;
    }
}

void Simulation::applyPressureMassPhysiscs(float dt)
{
    for (PressureSpringMassModel& model : SpringMassModels)
    {
        model.applyForces(MassPoints, dt);
    }
}

void Simulation::generateSprings(PressureSpringMassModel& model)
{
    Spring currSpringData = Spring();
    currSpringData.dampening = 200.0f;
    currSpringData.spring_const = 25000.0f;
    model.approximateArea(MassPoints);
    model.nRT = model.Area * 500.0f;

    for (int i = 0; i < model.MassPoints.size(); i++)
    {

        currSpringData.A = model.MassPoints[i];
        currSpringData.B = model.MassPoints[(i + 1) % model.MassPoints.size()];
        currSpringData.rest_length = 0.000625f * (MassPoints[i].position - MassPoints[i + 1].position).length();
        Springs.push_back(currSpringData);
        //std::cout << "spring appended";
        model.Springs.push_back(Springs.size() - 1);
    }
}
