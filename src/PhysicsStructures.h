#pragma once
#include "Vector.h"
#include <iostream>

const float PI = 3.14159;

struct MassPoint 
{
	Vector2 position = Vector2();
	Vector2 velocity = Vector2();
	float collideRadius = 5.0f;
	float mass = 5.0f;
	bool iscollide=false;

	bool isColliding(MassPoint& other)
	{
		float dist = (position - other.position).length();
		if (dist < collideRadius + other.collideRadius)
		{
			iscollide = true;
			other.iscollide = true;
			return true;
		}
		else
		{
			return false;
		}
	}

	void collide(MassPoint& other)
	{
		float dist = (position - other.position).length();
		float inside_dist = (collideRadius + other.collideRadius) - dist;
		Vector2 toOther = (other.position - position).normalized();
		//float to_move = inside_dist / 2.0f

		Vector2 temp = Vector2(velocity.x,velocity.y);
		velocity -= (position - other.position)
			* ((2 * other.mass) / (mass + other.mass))
			* ((velocity - other.velocity).dot(position - other.position)
			/ (position - other.position).length2());

		other.velocity -= (other.position - position)
			* ((2 * mass) / (mass + other.mass))
			* ((other.velocity - temp).dot(other.position - position) 
			/ (other.position - position).length2());

		position -= toOther * inside_dist / 2.0f;
		other.position += toOther * inside_dist / 2.0f;

	}

	void collide(Vector2 pos, Vector2 velocity, float r)
	{
		MassPoint mp;
		mp.position = pos;
		mp.velocity = velocity;
		mp.mass = 100000000000000000;
		mp.collideRadius = r;
		collide(mp);
	}
};

struct Spring 
{	
	//index
	int A; 
	int B;
	float dampening = 1.0f;
	float spring_const = 1.0f;
	float rest_length = 1.0f;
	float spring_force = 0.0f;
	Vector2 force_A = Vector2();
	Vector2 force_B = Vector2();

	void applyForces(std::vector<MassPoint>& masspoints, float dt) 
	{
		spring_force = (((masspoints[B].position - masspoints[A].position).length() - rest_length) * spring_const 
			+ ((masspoints[B].position - masspoints[A].position).normalized()).dot(masspoints[B].velocity - masspoints[A].velocity) * dampening);
		force_A = (masspoints[B].position - masspoints[A].position).normalized() * spring_force;
		force_B = (masspoints[A].position - masspoints[B].position).normalized() * spring_force;


		masspoints[A].velocity += force_A * dt / masspoints[A].mass;
		masspoints[B].velocity += force_B * dt / masspoints[B].mass;
	}
};

struct PressureSpringMassModel
{
	std::vector<int> MassPoints = {};
	std::vector<int> Springs = {};
	Vector2 center;
	float Pressure;
	float nRT;
	float Area;

	float calculateArea(const std::vector<MassPoint>& masspoints)
	{
		for (int i = 0; i < MassPoints.size(); i++)
		{
			center += masspoints[MassPoints[i]].position;
			// possible cause for problem;
			int j = MassPoints[(i + 1) % MassPoints.size()];
			Area += 0.5f * (masspoints[i].position.x * masspoints[j].position.y 
				- masspoints[j].position.x * masspoints[i].position.y);

		}

		center /= MassPoints.size();
		std::cout << Area << std::endl;
		return Area;
	}

	float approximateArea(const std::vector<MassPoint>& masspoints) {
		float radius;
		float smallestRadius = 1000;
		float biggestRadius = 0;

		for (int i = 0; i < MassPoints.size(); i++)
		{
			center += masspoints[MassPoints[i]].position;
		}
		center /= MassPoints.size();
		
		for (int i = 0; i < MassPoints.size(); i++)
		{
			// possible cause for problem (fucking unoptimized);
			int j = MassPoints[(i + 1) % MassPoints.size()];
			radius = (masspoints[MassPoints[i]].position - center).length();
			if (radius < smallestRadius) 
			{
				smallestRadius = radius;
			}
			if (radius > biggestRadius)
			{
				biggestRadius = radius;
			}

		}

		Area = smallestRadius * biggestRadius * PI;
		//std::cout << Area << std::endl;
		return Area;
		
	}

	void applyForces(std::vector<MassPoint>& masspoints, float dt)
	{

		approximateArea(masspoints);
		Pressure = nRT * 500.0f / Area;

		Vector2 Force;

		for (int i = 0; i < MassPoints.size(); i++)
		{
			Force = (masspoints[MassPoints[i]].position - center).normalized() * Pressure;
			masspoints[MassPoints[i]].velocity += Force * dt / masspoints[MassPoints[i]].mass;
		}
	}
};
