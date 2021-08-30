/*
 * Copyright (c) 2020 EKA2L1 Team
 * 
 * This file is part of EKA2L1 project.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <common/types.h>
#include <common/uid.h>

#include <cstdint>
#include <utils/des.h>

#include <vector>

namespace eka2l1::epoc::msv {
    enum change_notification_type {
        change_notification_type_entries_no = 0,
        change_notification_type_entries_created = 1,
        change_notification_type_entries_changed = 2,
        change_notification_type_entries_deleted = 3,
        change_notification_type_entries_moved = 4,

        change_notification_type_mtm_group_installed = 5,
        change_notification_type_mtm_group_uninstalled = 6,

        change_notification_type_store_deleted = 7,
        change_notification_type_close_session = 8,

        change_notification_type_index_loaded = 9,
        change_notification_type_index_load_failed = 10,

        change_notification_type_index_media_changed = 12,
        change_notification_type_index_media_unavailable = 13,
        change_notification_type_index_media_available = 14,
        change_notification_type_index_media_incorrect = 15,

        change_notification_type_msg_store_not_supported = 16,
        change_notification_type_msg_store_corrupt = 17,
        change_notification_type_refresh_msg_view = 18,
        change_notification_type_disk_unavail = 19,
        change_notification_type_unable_to_process_disk_nof = 20
    };

    struct entry {
        std::uint32_t id_;
        std::int32_t parent_id_;
        std::uint32_t service_id_;
        std::uint32_t related_id_;
        std::uint32_t visible_id_;
        std::int32_t pc_sync_count_;
        epoc::uid type_uid_;
        epoc::uid mtm_uid_;
        std::uint32_t data_ = 0;
        std::u16string description_;
        std::u16string details_;
        std::uint64_t time_;

        std::int32_t size_;
        std::int32_t error_;
        std::int32_t bio_type_;
        std::int32_t reserved_ = 0;

        std::int32_t mtm_datas_[3];

        std::vector<std::uint32_t> children_ids_;
        std::uint32_t emulator_flags_ = 0;

        enum emulator_flag {
            EMULATOR_FLAG_CHILDREN_LOOKED = 1 << 0
        };

        enum data_flag {
            DATA_FLAG_INCOMPLETE = 0x8,
            DATA_FLAG_NEW = 0x10,
            DATA_FLAG_UNREAD = 0x20,
            DATA_FLAG_INVISIBLE = 0x200,
            DATA_FLAG_MULTI_RECIPIENTS = 0x400,
            DATA_FLAG_SCHEDULED = 0x800,
            DATA_FLAG_READ_ONLY = 0x1000,
            DATA_FLAG_DELETED = 0x2000,
            DATA_FLAG_ATTACHMENT = 0x8000,
            DATA_FLAG_VISIBLE_FOLDER_FLAG = 0x04000000
        };

        bool children_looked_up() const {
            return emulator_flags_ & EMULATOR_FLAG_CHILDREN_LOOKED;
        }

        void children_looked_up(bool value) {
            emulator_flags_ &= ~EMULATOR_FLAG_CHILDREN_LOOKED;
            if (value) {
                emulator_flags_ |= EMULATOR_FLAG_CHILDREN_LOOKED;
            }
        }

        const bool visible() const {
            return !(data_ & DATA_FLAG_INVISIBLE);
        }

        void visible(const bool value) {
            data_ &= ~DATA_FLAG_INVISIBLE;
            if (!value) {
                data_ |= DATA_FLAG_INVISIBLE;
            }
        }

        const bool visible_folder() const {
            return data_ & DATA_FLAG_VISIBLE_FOLDER_FLAG;
        }

        void visible_folder(const bool value) {
            data_ &= ~DATA_FLAG_VISIBLE_FOLDER_FLAG;
            if (value) {
                data_ |= DATA_FLAG_VISIBLE_FOLDER_FLAG;
            }
        }
    };

    using msv_id = std::uint32_t;

    static constexpr std::uint32_t MTM_DEFAULT_SPECIFIC_UID = 0x10202D51;
    static constexpr std::uint32_t MTM_SERVICE_UID_ROOT = 0x10000F67;
    static constexpr std::uint32_t MSV_SERVICE_UID = 0x10000F68;
    static constexpr std::uint32_t MSV_FOLDER_UID = 0x10000F69;
    static constexpr std::uint32_t MSV_ROOT_ID_VALUE = 0x1000;
    static constexpr std::uint32_t MSV_FIRST_FREE_ENTRY_ID = 0x100000;
}