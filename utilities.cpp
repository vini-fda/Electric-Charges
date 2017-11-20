#include "utilities.hpp"
void Line::setVertexPosition(sf::Vector2f p, int b) { vArray[b%2].position = p; }
void Line::setVerticesPosition(sf::Vector2f p0, sf::Vector2f p1) { vArray[0].position = p0; vArray[1].position = p1; }
void Line::setColor(sf::Color color) { vArray[0].color = vArray[1].color = color; }
const sf::Vector2f Line::asVector() { return vArray[1].position - vArray[0].position; }
const sf::VertexArray& Line::getVertexArray() { return vArray; }
//Other utilities
float length(sf::Vector2f v){
  return sqrt(v.x*v.x + v.y*v.y);
}
float distance(sf::Vector2f v, sf::Vector2f w)
{
  return length(v - w);
}
sf::Vector2f unit(sf::Vector2f v)
{
  return (length(v) == 0) ? v : v/length(v);
}