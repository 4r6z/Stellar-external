#pragma once

#ifndef PART_HPP
#define PART_HPP

#include "instance.hpp"
#include "../helpers/math.hpp"

class Part : public Instance {
public:
    Vector3 position() {
        if (!self)
            return Vector3();

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return Vector3();

        return read<Vector3>(primitive + stellar::offsets::position);
    }

    Matrix3 rotation() {
        if (!self)
            return Matrix3();

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return Matrix3();

        return read<Matrix3>(primitive + stellar::offsets::rotation);
    }

    Vector3 velocity() {
        if (!self)
            return Vector3();

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return Vector3();

        return read<Vector3>(primitive + stellar::offsets::velocity);
    }

    Vector3 size() {
        if (!self)
            return Vector3();

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return Vector3();

        return read<Vector3>(primitive + stellar::offsets::part_size);
    }

    void set_velocity(const Vector3& new_velocity) {
        if (!self)
            return;

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return;

        write<Vector3>(primitive + stellar::offsets::velocity, new_velocity);
    }

    Vector3 get_velocity() {
        if (!self)
            return Vector3();

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return Vector3();

        return read<Vector3>(primitive + stellar::offsets::velocity);
    }

    void write_velocity(const Vector3& new_velocity) {
        if (!self)
            return;

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return;

        write<Vector3>(primitive + stellar::offsets::velocity, new_velocity);
    }

    Vector3 get_pos() {
        if (!self)
            return Vector3();

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return Vector3();

        return read<Vector3>(primitive + stellar::offsets::position);
    }

    void write_position(const Vector3& new_position) {
        if (!self)
            return;

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return;

        write<Vector3>(primitive + stellar::offsets::position, new_position);
    }


    Vector3 set_part_pos(Vector3& arghere) {

        Vector3 res{};

        std::uint64_t primitive = read<std::uint64_t>(self + stellar::offsets::primitive);

        if (!primitive)
            return res;


        int count = 0;

        while (true) {

            count = count + 1;

            write<Vector3>(primitive + stellar::offsets::position, arghere);

            if (count == 10000) {
                break;
            }

        }

    }


    Vector3 move_direction() {
        if (!self)
            return Vector3();

        Vector3 direction = read<Vector3>(self + stellar::offsets::move_direction);

        // Clamp the direction values to the range [-1, 1]
        if (direction.x < -1 || direction.x > 1)
            direction.x = 0;

        if (direction.y < -1 || direction.y > 1)
            direction.y = 0;

        if (direction.z < -1 || direction.z > 1)
            direction.z = 0;


        return direction;
    }
};

#endif // PART_HPP
