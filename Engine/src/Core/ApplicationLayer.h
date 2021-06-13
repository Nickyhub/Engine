#pragma once
#include "Layer.h"


namespace Engine {

	class ApplicationLayer : public Layer
	{
	public:
		ApplicationLayer();
		void OnAttach() override;
		void OnEvent(const Event& e) override;
		void OnUpdate() override;
		void OnDetach() override;
		bool IsActive() override;
		void SetActive(bool active) override;
	private:
		bool m_Active;
		std::string m_Name;
		
	};
}


