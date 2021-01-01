/* (c) 2020-2021 Галушин Павел Викторович, galushin@gmail.com

Данный файл -- часть библиотеки SAGA.

SAGA -- это свободной программное обеспечение: вы можете перераспространять ее и/или изменять ее
на условиях Стандартной общественной лицензии GNU в том виде, в каком она была опубликована Фондом
свободного программного обеспечения; либо версии 3 лицензии, либо (по вашему выбору) любой более
поздней версии.

Это программное обеспечение распространяется в надежде, что оно будет полезной, но БЕЗО ВСЯКИХ
ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
Подробнее см. в Стандартной общественной лицензии GNU.

Вы должны были получить копию Стандартной общественной лицензии GNU вместе с этим программным
обеспечение. Если это не так, см. https://www.gnu.org/licenses/.
*/

#ifndef Z_SAGA_MATH_PROBABILITY_HPP_INCLUDED
#define Z_SAGA_MATH_PROBABILITY_HPP_INCLUDED

/** @file saga/math/probability.hpp
 @brief Класс для представления вероятности и вспомогательные возможности
*/

#include <cassert>

#include <stdexcept>
#include <utility>

namespace saga
{
    /// @brief Политика проверки значений для класса вероятностей на основе макроса cassert
    class probability_policy_cassert
    {
    public:
        /** @brief Проверка значения
        @param value
        @pre <tt>0.0 <= value && value <= 1.0</tt>
        @return value
        */
        template <class RealType>
        static RealType enforce(RealType value)
        {
            assert(RealType(0.0) <= value && value <= RealType(1.0));
            return value;
        }

    protected:
        ~probability_policy_cassert() = default;
    };

    /// @brief Политика проверки значений для класса вероятностей, порождающий исключения
    class probability_policy_throw
    {
    public:
        /** @brief Проверка значения
        @param value
        @return value
        @throw std::logic_error, если не выполняется условие <tt>0.0 <= value && value <= 1.0</tt>
        */
        template <class RealType>
        static RealType enforce(RealType value)
        {
            if(value < RealType(0.0) || RealType(1.0) < value)
            {
                throw std::logic_error("Incorrect probability value");
            }

            return value;
        }

    protected:
        ~probability_policy_throw() = default;
    };

    /** @brief Класс для представления вероятности
    @tparam RealType тип для представления вещественного значения
    @tparam CheckPolicy политика проверка значений
    @todo Проверка того, что тип может быть использован для представления вещественных чисел
    @todo Политика проверки, которая не выполняет никаких проверок
    */
    template <class RealType, class CheckPolicy = probability_policy_cassert>
    class probability
     : protected CheckPolicy
    {
    public:
        // Типы
        /// @brief Тип значения
        using value_type = RealType;

        /// @brief Политика проверки значений
        using check_policy = CheckPolicy;

        // Создание, копирование, уничтожение
        /// @brief Конструктор без параметов
        probability() = default;

        /** @brief Конструктор с заданным значением
        @param value значение

        Значение проверяется с помощью метода <tt>check_policy::enforce</tt>
        */
        explicit probability(value_type value)
         : value_(check_policy::enforce(std::move(value)))
        {}

        // Свойства
        /** @brief Доступ к значению
        @return Значение вероятности, которое было задано в конструкторе или при последнем
        присваивании.
        */
        value_type const & value() const
        {
            return this->value_;
        }

    private:
        value_type value_ = RealType(0.0);
    };
}
// namespace saga

#endif
// Z_SAGA_MATH_PROBABILITY_HPP_INCLUDED
