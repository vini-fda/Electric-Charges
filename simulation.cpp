#include "simulation.hpp"

Simulation::Simulation()
    : mainWindow(sf::VideoMode(1024, 1024), "Electric"),
      mainView(sf::Vector2f(0, 0), sf::Vector2f(800, 800))

{
    lockView = false;
    viewSpeed = 0.8f;
    currentTool = charge;
    if (!courierPrime.loadFromFile("Courier Prime.ttf")) {
        // Hold up, mah boi
        std::cout << "Font file was unable to be loaded" << std::endl;
        mainWindow.close();
    }
    chargeCount.setString("Charges: 0");
    chargeCount.setFont(courierPrime);
    chargeCount.setCharacterSize(30);
    chargeCount.setPosition(mainWindow.mapPixelToCoords(sf::Vector2i(0, 0)));
}
void Simulation::run()
{
    while (mainWindow.isOpen()) {
        dt = mainClock.restart();
        mainMousePos =
            mainWindow.mapPixelToCoords(sf::Mouse::getPosition(mainWindow));
        processEvents();
        processRealTimeInput();
        update();
        render();
    }
}
void Simulation::processEvents()
{
    sf::Event event;
    while (mainWindow.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            mainWindow.close();
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                switch (currentTool) {
                case force:
                    forceTool.usePrimary(true, chargeVector, mainMousePos);
                    break;
                case charge:
                    chargeCreatorTool.usePrimary(true, chargeVector,
                                                 mainMousePos);
                    break;
                case follow: {
                    followTool.usePrimary(true, chargeVector, mainMousePos);
                    lockView = followTool.focusViewOnCharge(mainView);
                    break;
                }
                }
            }
        }
        if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                switch (currentTool) {
                case force:
                    forceTool.usePrimary(false, chargeVector, mainMousePos);
                    break;
                case charge: {
                    chargeCreatorTool.usePrimary(false, chargeVector,
                                                 mainMousePos);

                    chargeCount.setString("Charges: " +
                                          std::to_string(chargeVector.size()));
                    break;
                }
                }
            }
        }
        if (event.type == sf::Event::Resized) {
            // Update the mainView to the new size of the mainWindow
            mainView.setSize(event.size.width, event.size.height);
            mainWindow.setView(mainView);
        }
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
            case sf::Keyboard::C: currentTool = charge; break;
            case sf::Keyboard::F: currentTool = force; break;
            case sf::Keyboard::G:
                currentTool = follow;
                break;
            // case sf::Keyboard::X: currentTool = trash; break;
            case sf::Keyboard::L: lockView = false; break;
            }
        }
    }
}
void handleKeyboardEvent(sf::Keyboard::Key key, bool isPressed) {}
void handleMouseEvent(sf::Mouse::Button button, bool isPressed) { /*
     switch (button) {
     case sf::Mouse::Left: break;
   }*/}

void Simulation::processRealTimeInput()
{
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        if (currentTool == force) {
            forceTool.updateInitialPos();
            forceTool.setCurrentPos(mainMousePos);
        }
        if (currentTool == charge) {
            chargeCreatorTool.setCurrentPos(mainMousePos);
        }
    }
    if (!lockView) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            mainView.move(-viewSpeed, 0);
            mainWindow.setView(mainView);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            mainView.move(viewSpeed, 0);
            mainWindow.setView(mainView);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            mainView.move(0, -viewSpeed);
            mainWindow.setView(mainView);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            mainView.move(0, viewSpeed);
            mainWindow.setView(mainView);
        }
    } /*else {
        if (followCharge != nullptr) {
            mainView.setCenter(followCharge->getPosition());
            mainWindow.setView(mainView);
        }
    }*/
}
void Simulation::update()
{
    // Update physics
    // Forces
    for (auto charge : chargeVector) {
        charge->setForce(sf::Vector2f(0, 0));
    }
    forceTool.applyForce();
    using chargePtrIter = std::vector<std::shared_ptr<Charge>>::iterator;
    for (chargePtrIter s = chargeVector.begin(); s != chargeVector.end(); s++) {

        // Collision
        sf::Vector2f sPos = (*s)->getPosition();
        (*s)->setIsCursorOn(distance(sPos, mainMousePos) < (*s)->getRadius());
        if ((*s)->getIsCursorOn()) {
            (*s)->setFillColor(sf::Color::Red);
        } else
            (*s)->setFillColor(sf::Color::White);
        for (chargePtrIter r = s; r != chargeVector.end(); r++) {
            if (detectChargeChargeCollision(*(*r), *(*s), dt.asSeconds())) {
                std::cout << "COLLISION" << std::endl;
                sf::Vector2f velDiff =
                    (*r)->getVelocity() - (*s)->getVelocity();
                sf::Vector2f normal =
                    unit((*s)->getPosition() -
                         (*r)->getPosition()); // Points from (*r) to (*s)
                float reducedMass = ((*r)->getMass() * (*s)->getMass()) /
                                    ((*r)->getMass() + (*s)->getMass());
                sf::Vector2f J =
                    2 * reducedMass * dotProduct(velDiff, normal) * normal;
                (*r)->incrementForce(-J / dt.asSeconds());
                (*s)->incrementForce(J / dt.asSeconds());
            }
        }
    }
    // Velocity
    // Position
    for (auto charge : chargeVector) {
        charge->incrementVelocity(dt.asSeconds() * charge->getForce() /
                                  charge->getMass());
        charge->setPosition(charge->getPosition() +
                            charge->getVelocity() * dt.asSeconds());
    }

    // WHERE  ELSE TO PUT THIS?
    if (lockView) {
        followTool.focusViewOnCharge(mainView);
        mainWindow.setView(mainView);
    }
}
void Simulation::render()
{
    mainWindow.clear();
    // Draw charges
    for (auto s : chargeVector) {
        mainWindow.draw(*s);
        if (s->getIsCursorOn()) {
            mainWindow.draw(s->velocityLine().getVertexArray());
        }
    }
    forceTool.draw(mainWindow);
    chargeCreatorTool.draw(mainWindow);

    // Draw Text
    mainWindow.draw(chargeCount);
    mainWindow.display();
}