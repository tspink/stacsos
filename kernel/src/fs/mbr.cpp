#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/dev/device-manager.h>
#include <stacsos/kernel/dev/storage/mbr.h>
#include <stacsos/kernel/dev/storage/partitioned-device.h>

using namespace stacsos::kernel::dev::storage;

struct partition_table_entry {
	u8 drive_attributes;
	u8 chs_first[3];
	u8 partition_type;
	u8 chs_last[3];
	u32 lba_partition_start;
	u32 nr_sectors;
} __packed;

static_assert(sizeof(partition_table_entry) == 16, "partition table entry size incorrect");

void mbr::scan()
{
	u8 *buffer = new u8[512];
	parent().read_blocks_sync(buffer, 0, 1);

	const partition_table_entry *ptr = (const partition_table_entry *)&buffer[0x1be];
	dprintf("mbr: partitions:\n");
	for (int i = 0; i < 4; i++) {
		if (ptr[i].partition_type == 6) {
			dprintf("  partition #%d: start=%lu, count=%lu\n", i, ptr[i].lba_partition_start, ptr[i].nr_sectors);
			device_manager::get().register_device(*new partition(parent(), ptr[i].lba_partition_start, ptr[i].nr_sectors));
		}
	}

	delete[] buffer;
}
