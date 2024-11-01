#pragma once

#include "FieldEntry.h"
#include <cheat-base/config/converters.h>
#include <type_traits>

namespace config::internal
{
	namespace CHECK
	{
		struct No {};
		template<typename T, typename Arg> No operator== (const T&, const Arg&);

		template<typename T, typename Arg = T>
		struct EqualExists
		{
			enum { value = !std::is_same<decltype(std::declval<T>() == std::declval<Arg>()), No>::value };
		};

		// Helper to check if T has a member named 'value'
		template<typename T, typename = void>
		struct has_value_member : std::false_type {};

		template<typename T>
		struct has_value_member<T, std::void_t<decltype(std::declval<T>().value)>> : std::true_type {};
	}

	template<typename T>
	class FieldSerialize : public FieldEntry
	{
	public:
		FieldSerialize(const std::string& friendlyName, const std::string& name, const std::string& sectionName, const T& defaultValue, bool multiProfile = false) :
			FieldEntry(friendlyName, name, sectionName, multiProfile), m_Value(defaultValue), m_DefaultValue(defaultValue) { }

		nlohmann::json ToJson() override
		{
			if constexpr (CHECK::EqualExists<T>::value)
			{
				if (m_Value == m_DefaultValue)
					return {};
			}

			// Ensure T can be serialized to JSON
			if constexpr (CHECK::has_value_member<T>::value)
			{
				return converters::ToJson(m_Value);
			}
			else
			{
				// Handle the case where T does not have a value member
				static_assert(CHECK::has_value_member<T>::value, "T must have a value member or be serializable to JSON.");
			}
		}

		void FromJson(const nlohmann::json& jObject) override
		{
			if (jObject.empty())
			{
				m_Value = m_DefaultValue;
				return;
			}

			converters::FromJson(m_Value, jObject);
		}
		
		void Reset() override
		{
			m_Value = m_DefaultValue;
		}

		T m_Value;
		T m_DefaultValue;
	};
}
