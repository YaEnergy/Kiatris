#pragma once

class Scene
{
	public:
		Scene() = default;
		
		virtual void Init()
		{

		}
		
		virtual void Update()
		{

		}

		virtual void Draw()
		{

		}

		virtual void Destroy()
		{

		}

		~Scene()
		{
			Destroy();
		}
};