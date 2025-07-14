#pragma once

namespace punk
{
    class data_storage_t
    {
    public:
        virtual ~data_storage_t() = default;
        data_storage_t(data_storage_t const&) = delete;
        data_storage_t& operator=(data_storage_t const&) = delete;
        data_storage_t(data_storage_t&&) = delete;
        data_storage_t& operator=(data_storage_t&&) = delete;

    public:

    };
}