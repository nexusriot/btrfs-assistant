<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>Btrfs</name>
    <message>
        <location filename="../src/util/Btrfs.cpp" line="115"/>
        <source>Failed to create the snapshot</source>
        <translation>创建快照失败</translation>
    </message>
    <message>
        <location filename="../src/util/Btrfs.cpp" line="380"/>
        <source>You cannot restore to the root of the partition</source>
        <translation>无法恢复到分区根目录</translation>
    </message>
    <message>
        <location filename="../src/util/Btrfs.cpp" line="407"/>
        <source>Failed to make a backup of target subvolume</source>
        <translation>备份目标子卷失败</translation>
    </message>
    <message>
        <location filename="../src/util/Btrfs.cpp" line="428"/>
        <source>Failed to restore subvolume!</source>
        <translation>恢复子卷失败！</translation>
    </message>
    <message>
        <location filename="../src/util/Btrfs.cpp" line="429"/>
        <source>Snapshot restore failed.  Please verify the status of your system before rebooting</source>
        <translation>快照恢复失败。请在重启前确认系统状态</translation>
    </message>
    <message>
        <location filename="../src/util/Btrfs.cpp" line="444"/>
        <source>The restore was successful but the migration of the nested subvolumes failed</source>
        <translation>恢复成功，但嵌套子卷迁移失败</translation>
    </message>
    <message>
        <location filename="../src/util/Btrfs.cpp" line="445"/>
        <source>Please migrate the those subvolumes manually</source>
        <translation>请手动迁移这些子卷</translation>
    </message>
    <message>
        <location filename="../src/util/Btrfs.cpp" line="572"/>
        <source>UUID </source>
        <translation>UUID </translation>
    </message>
</context>
<context>
    <name>BtrfsStatus</name>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="16"/>
        <location filename="../src/util/BtrfsStatus.cpp" line="55"/>
        <source>no stats available</source>
        <translation>无可用统计信息</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="19"/>
        <source>no errors found</source>
        <translation>未发现错误</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="22"/>
        <source>running</source>
        <translation>正在运行</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="25"/>
        <source>finished</source>
        <translation>已完成</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="28"/>
        <source>canceled</source>
        <translation>已取消</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="31"/>
        <source>aborted</source>
        <translation>已中止</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="34"/>
        <source>interrupted</source>
        <translation>已中断</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="64"/>
        <source>UUID:</source>
        <translation>UUID：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="65"/>
        <source>Scrub started:</source>
        <translation>校验（Scrub）开始时间：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="66"/>
        <source>Scrub resumed:</source>
        <translation>校验（Scrub）恢复时间：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="67"/>
        <source>Status:</source>
        <translation>状态：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="68"/>
        <source>Duration:</source>
        <translation>持续时间：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="69"/>
        <source>Time left:</source>
        <translation>剩余时间：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="70"/>
        <source>ETA:</source>
        <translation>预计完成时间：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="71"/>
        <source>Total to scrub:</source>
        <translation>待校验总量：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="72"/>
        <source>Bytes scrubbed:</source>
        <translation>已校验数据量：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="73"/>
        <source>Rate:</source>
        <translation>速率：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="74"/>
        <source>Error summary:</source>
        <translation>错误摘要：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="75"/>
        <source>read_errors:</source>
        <translation>读取错误：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="76"/>
        <source>csum_errors:</source>
        <translation>校验和错误：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="77"/>
        <source>verify_errors:</source>
        <translation>验证错误：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="78"/>
        <source>super_errors:</source>
        <translation>超级块错误：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="79"/>
        <source>malloc_errors:</source>
        <translation>内存分配错误：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="80"/>
        <source>uncorrectable_errors:</source>
        <translation>不可修复错误：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="81"/>
        <source>unverified_errors:</source>
        <translation>未验证错误：</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="111"/>
        <source>No balance operation found on %1</source>
        <translation>“%1”上没有正在运行的均衡（Balance）操作</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="116"/>
        <source>Balance operation on %1 is running</source>
        <translation>“%1”上的均衡（Balance）正在运行</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="121"/>
        <source>Balance operation on %1 is paused</source>
        <translation>“%1”上的均衡（Balance）已暂停</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="126"/>
        <source>Done, relocated %1 of %2 chunks</source>
        <translation>已完成，重定位了 %1/%2 个区块</translation>
    </message>
    <message>
        <location filename="../src/util/BtrfsStatus.cpp" line="131"/>
        <source>%1 of about %2 chunks balanced (%3 considered), %4% left</source>
        <translation>已完成约 %2 个区块中的 %1 个均衡（Balance）（已检查 %3 个），剩余 %4%</translation>
    </message>
</context>
<context>
    <name>Cli</name>
    <message>
        <location filename="../src/ui/Cli.cpp" line="34"/>
        <location filename="../src/ui/Cli.cpp" line="52"/>
        <source>You must run this application as root</source>
        <translation>必须以 root 身份运行此应用程序</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="61"/>
        <source>Failed to parse snapshot list</source>
        <translation>解析快照列表失败</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="69"/>
        <source>This is not a snapshot that can be restored by this application</source>
        <translation>此快照无法由本应用程序恢复</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="78"/>
        <source>Source snapshot not found</source>
        <translation>未找到源快照</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="83"/>
        <source>Snapshot subvolume not found</source>
        <translation>未找到快照子卷</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="92"/>
        <source>Target not found</source>
        <translation>未找到目标</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="96"/>
        <source>Restoring snapshot %1</source>
        <translation>正在恢复快照 %1</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="103"/>
        <source>Snapshot restoration complete.</source>
        <translation>快照恢复完成。</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="104"/>
        <source>A copy of the original subvolume has been saved as </source>
        <translation>原始子卷的副本已保存为：</translation>
    </message>
    <message>
        <location filename="../src/ui/Cli.cpp" line="105"/>
        <source>Please reboot immediately
</source>
        <translation>请立即重启
</translation>
    </message>
</context>
<context>
    <name>DiffViewer</name>
    <message>
        <location filename="../src/ui/DiffViewer.ui" line="14"/>
        <source>Dialog</source>
        <translation>对话框</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.ui" line="83"/>
        <source>Select a snapshot from the left to see the diff</source>
        <translation>从左侧选择快照以查看差异</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.ui" line="116"/>
        <source>Restore</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.ui" line="123"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="14"/>
        <source>Diff Viewer</source>
        <translation>差异查看器</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="29"/>
        <source>Confirm</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="29"/>
        <source>Are you sure you want to restore this the file over the current file?</source>
        <translation>确定要用此文件覆盖当前文件吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="38"/>
        <source>Restore Failed</source>
        <translation>恢复失败</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="38"/>
        <source>The file failed to restore</source>
        <translation>文件恢复失败</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="42"/>
        <source>Restore File</source>
        <translation>恢复文件</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="42"/>
        <source>The file was successfully restored</source>
        <translation>文件恢复成功</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="63"/>
        <source>Num</source>
        <comment>The number associated with a snapshot</comment>
        <translation>编号</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="64"/>
        <source>Date/Time</source>
        <translation>日期/时间</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="65"/>
        <source>Root Path</source>
        <translation>根路径</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="66"/>
        <source>File Path</source>
        <translation>文件路径</translation>
    </message>
    <message>
        <location filename="../src/ui/DiffViewer.cpp" line="121"/>
        <source>There are no differences between the selected files</source>
        <translation>所选文件之间没有差异</translation>
    </message>
</context>
<context>
    <name>FileBrowser</name>
    <message>
        <location filename="../src/ui/FileBrowser.ui" line="14"/>
        <source>Dialog</source>
        <translation>对话框</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.ui" line="70"/>
        <source>Show Diff</source>
        <translation>显示差异</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.ui" line="83"/>
        <location filename="../src/ui/FileBrowser.cpp" line="93"/>
        <location filename="../src/ui/FileBrowser.cpp" line="113"/>
        <source>Restore File</source>
        <translation>恢复文件</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.ui" line="96"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="41"/>
        <source>Snapshot File Viewer</source>
        <translation>快照文件查看器</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="53"/>
        <source>File Viewer</source>
        <translation>文件查看器</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="70"/>
        <source>Diff File</source>
        <translation>比较文件</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="71"/>
        <source> is a directory, only files can be diffed</source>
        <translation> 是目录，只能比较文件</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="94"/>
        <source> is a directory, only files can be restored</source>
        <translation> 是目录，只能恢复文件</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="98"/>
        <source>Confirm</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="98"/>
        <source>Are you sure you want to restore this the file over the current file?</source>
        <translation>确定要用此文件覆盖当前文件吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="109"/>
        <source>Restore Failed</source>
        <translation>恢复失败</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="109"/>
        <source>The file failed to restore</source>
        <translation>文件恢复失败</translation>
    </message>
    <message>
        <location filename="../src/ui/FileBrowser.cpp" line="113"/>
        <source>The file was successfully restored</source>
        <translation>文件恢复成功</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="14"/>
        <source>BTRFS-Assistant</source>
        <translation>Btrfs 助手</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="31"/>
        <source>Overview</source>
        <translation>概览</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="67"/>
        <location filename="../src/ui/MainWindow.ui" line="1577"/>
        <source>Scrub</source>
        <translation>校验（Scrub）</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="108"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Perform scrub on device.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;对设备执行校验（Scrub）。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="111"/>
        <location filename="../src/ui/MainWindow.ui" line="195"/>
        <location filename="../src/ui/MainWindow.cpp" line="159"/>
        <location filename="../src/ui/MainWindow.cpp" line="179"/>
        <source>Start</source>
        <translation>开始</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="126"/>
        <source>No scrub ran.</source>
        <translation>尚未执行校验（Scrub）。</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="151"/>
        <location filename="../src/ui/MainWindow.ui" line="1503"/>
        <source>Balance</source>
        <translation>均衡（Balance）</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="192"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Perform full balance on device.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;对设备执行完整均衡（Balance）。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="214"/>
        <source>No balance ran.</source>
        <translation>尚未执行均衡（Balance）。</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="236"/>
        <source>Internal Filesystem Statistics</source>
        <translation>内部文件系统统计</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="242"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Utilization percentage of system chunks in the allocated space.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;已分配空间中系统区块的利用率。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="252"/>
        <source>Data:</source>
        <translation>数据：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="259"/>
        <source>Metadata:</source>
        <translation>元数据：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="266"/>
        <source>System:</source>
        <translation>系统：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="273"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Utilization percentage of metadata chunks in the allocated space.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;已分配空间中元数据区块的利用率。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="283"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Utilization percentage of file data chunks in the allocated space.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;已分配空间中文件数据区块的利用率。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="302"/>
        <source>Information</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="308"/>
        <source>Used:</source>
        <translation>已用：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="315"/>
        <location filename="../src/ui/MainWindow.ui" line="392"/>
        <source>Estimate of the amount of data that can still be written to this FS, based on the current usage profile.</source>
        <translation>根据当前使用配置估算仍可写入此文件系统的数据量。</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="325"/>
        <location filename="../src/ui/MainWindow.ui" line="402"/>
        <source>Minimum amount of data that you can expect to be able to get onto the filesystem. </source>
        <translation>预计至少还能写入文件系统的数据量。</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="345"/>
        <source>Allocated:  </source>
        <translation>已分配：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="359"/>
        <source>Filesystem Size: </source>
        <translation>文件系统大小：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="395"/>
        <source>Free (Estimated): </source>
        <translation>可用（估算）：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="405"/>
        <source>Free (Minimum): </source>
        <translation>可用（最低）：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="421"/>
        <source>Volume Selection</source>
        <translation>卷选择</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="427"/>
        <source>Filesystem:</source>
        <translation>文件系统：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="463"/>
        <source>Enable Quotas</source>
        <translation>启用配额</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="470"/>
        <source>Refresh Btrfs Data</source>
        <translation>刷新 Btrfs 数据</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="485"/>
        <source>Subvolumes</source>
        <translation>子卷</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="523"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Include children subvolumes of .snapshots and timeshift folders.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;包含 .snapshots 和 timeshift 目录下的子卷。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="526"/>
        <source>Include Timeshift and Snapper Snapshots</source>
        <translation>包含 Timeshift 和 Snapper 快照</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="536"/>
        <source>Include Container Subvolumes</source>
        <translation>包含容器子卷</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="557"/>
        <source>Filter...</source>
        <translation>筛选...</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="583"/>
        <source>Restore Backup</source>
        <translation>恢复备份</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="603"/>
        <location filename="../src/ui/MainWindow.ui" line="905"/>
        <source>Browse</source>
        <translation>浏览</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="623"/>
        <location filename="../src/ui/MainWindow.ui" line="786"/>
        <location filename="../src/ui/MainWindow.ui" line="1070"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="643"/>
        <location filename="../src/ui/MainWindow.ui" line="812"/>
        <location filename="../src/ui/MainWindow.ui" line="951"/>
        <source>Refresh</source>
        <translation>刷新</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="690"/>
        <location filename="../src/ui/MainWindow.cpp" line="861"/>
        <source>Snapper</source>
        <translation>Snapper</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="712"/>
        <source>New/Delete</source>
        <translation>新建/删除</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="733"/>
        <location filename="../src/ui/MainWindow.ui" line="1040"/>
        <source>Select config: </source>
        <translation>选择配置：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="766"/>
        <location filename="../src/ui/MainWindow.ui" line="1063"/>
        <source>New</source>
        <translation>新建</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="849"/>
        <source>Browse/Restore</source>
        <translation>浏览/恢复</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="870"/>
        <source>Select target: </source>
        <translation>选择目标：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="925"/>
        <source>Restore</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="992"/>
        <source>Snapper Settings</source>
        <translation>Snapper 设置</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1090"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1100"/>
        <location filename="../src/ui/MainWindow.ui" line="1163"/>
        <source>Config Information</source>
        <translation>配置信息</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1106"/>
        <location filename="../src/ui/MainWindow.ui" line="1169"/>
        <source>Config name: </source>
        <translation>配置名称：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1133"/>
        <location filename="../src/ui/MainWindow.ui" line="1196"/>
        <source>Backup path: </source>
        <translation>备份路径：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1232"/>
        <source>Snapshot Retention</source>
        <translation>快照保留策略</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1238"/>
        <location filename="../src/ui/MainWindow.ui" line="1261"/>
        <location filename="../src/ui/MainWindow.ui" line="1275"/>
        <location filename="../src/ui/MainWindow.ui" line="1282"/>
        <location filename="../src/ui/MainWindow.ui" line="1353"/>
        <source>Save: </source>
        <translation>保留：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1251"/>
        <location filename="../src/ui/MainWindow.ui" line="1289"/>
        <source>Timeline snapshots are taken hourly, the cleanup job reduces the snapshots in accordance with the below settings</source>
        <translation>时间线快照每小时创建一次，清理任务会根据以下设置减少快照数量</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1254"/>
        <source>Enable timeline snapshots</source>
        <translation>启用时间线快照</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1268"/>
        <source>Daily</source>
        <translation>每日</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1308"/>
        <source>Yearly</source>
        <translation>每年</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1315"/>
        <source>Monthly</source>
        <translation>每月</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1325"/>
        <source>Hourly</source>
        <translation>每小时</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1332"/>
        <source>Weekly</source>
        <translation>每周</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1339"/>
        <source>Save:</source>
        <translation>保留：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1346"/>
        <source>Number</source>
        <translation>数量</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1382"/>
        <source>systemd Unit Settings</source>
        <translation>systemd 单元设置</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1388"/>
        <source>Snapper timeline enabled</source>
        <translation>启用 Snapper 时间线</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1401"/>
        <source>Snapper cleanup enabled</source>
        <translation>启用 Snapper 清理</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1414"/>
        <source>Snapper boot enabled</source>
        <translation>启用 Snapper 启动快照</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1433"/>
        <source>Apply systemd changes</source>
        <translation>应用 systemd 更改</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1461"/>
        <source>Btrfs maintenance</source>
        <translation>Btrfs 维护</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1531"/>
        <location filename="../src/ui/MainWindow.ui" line="1621"/>
        <location filename="../src/ui/MainWindow.ui" line="1695"/>
        <source>Frequency: </source>
        <translation>频率：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1541"/>
        <location filename="../src/ui/MainWindow.ui" line="1628"/>
        <location filename="../src/ui/MainWindow.ui" line="1702"/>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1548"/>
        <location filename="../src/ui/MainWindow.ui" line="1635"/>
        <location filename="../src/ui/MainWindow.ui" line="1709"/>
        <source>Mountpoints: </source>
        <translation>挂载点：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1651"/>
        <source>Defrag</source>
        <translation>碎片整理</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1747"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1767"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Update Btrfs Maintenance config file and call service to load new settings.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;更新 Btrfs Maintenance 配置文件，并调用服务加载新设置。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.ui" line="1770"/>
        <source>Apply</source>
        <translation>应用</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="59"/>
        <source>The application must be run as the superuser(root)</source>
        <translation>必须以超级用户（root）身份运行此应用程序</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="87"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="308"/>
        <source>You have lots of free space, did you overbuy?</source>
        <translation>可用空间很多，是不是买大了？</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="310"/>
        <source>Situation critical!  Time to delete some data or buy more disk</source>
        <translation>情况危急！该删除一些数据或购买更大的磁盘了</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="312"/>
        <source>Your disk space is well utilized</source>
        <translation>磁盘空间利用率良好</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="364"/>
        <location filename="../src/ui/MainWindow.cpp" line="414"/>
        <source>Number</source>
        <comment>The number associated with a snapshot</comment>
        <translation>编号</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="365"/>
        <location filename="../src/ui/MainWindow.cpp" line="418"/>
        <source>Date/Time</source>
        <translation>日期/时间</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="366"/>
        <location filename="../src/ui/MainWindow.cpp" line="419"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="367"/>
        <source>Cleanup</source>
        <translation>清理策略</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="368"/>
        <location filename="../src/ui/MainWindow.cpp" line="421"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="416"/>
        <source>Subvolume</source>
        <translation>子卷</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="532"/>
        <source>This is not a snapshot that can be restored by this application</source>
        <translation>此快照无法由本应用程序恢复</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="542"/>
        <source>Snapshot subvolume not found</source>
        <translation>未找到快照子卷</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="552"/>
        <source>Target not found</source>
        <translation>未找到目标</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="558"/>
        <source>Warning subvolid mount detected!</source>
        <translation>警告：检测到按 subvolid 挂载！</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="559"/>
        <source>It appears you are currently mounting by subvolid.  Doing a restore in this case may not produce the expected outcome.  It is highly recommended you switch to mounting by subvolume path before proceeding!</source>
        <translation>当前似乎正在按 subvolid 挂载。在这种情况下执行恢复可能无法得到预期结果。强烈建议在继续前改为按子卷路径挂载！</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="564"/>
        <source>Are you sure you want to restore </source>
        <translation>确定要恢复 </translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="564"/>
        <source> to </source>
        <comment>as in from/to</comment>
        <translation> 到 </translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="578"/>
        <source>Snapshot Restore</source>
        <translation>快照恢复</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="579"/>
        <source>Snapshot restoration complete.</source>
        <translation>快照恢复完成。</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="579"/>
        <location filename="../src/ui/MainWindow.cpp" line="1534"/>
        <source>A copy of the original subvolume has been saved as </source>
        <translation>原始子卷的副本已保存为：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="580"/>
        <location filename="../src/ui/MainWindow.cpp" line="1535"/>
        <source>Please reboot immediately</source>
        <translation>请立即重启</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="778"/>
        <source>No config selected</source>
        <translation>未选择配置</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="784"/>
        <source>Please Confirm</source>
        <translation>请确认</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="785"/>
        <source>Are you sure you want to delete </source>
        <translation>确定要删除 </translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="785"/>
        <source>This action cannot be undone</source>
        <translation>此操作无法撤销</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="816"/>
        <source>No btrfs subvolumes found</source>
        <translation>未找到 Btrfs 子卷</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="842"/>
        <source>Failed to save changes</source>
        <translation>保存更改失败</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="861"/>
        <source>Changes saved</source>
        <translation>更改已保存</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="874"/>
        <source>Please enter a valid name</source>
        <translation>请输入有效名称</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="880"/>
        <source>That name is already in use!</source>
        <translation>该名称已被使用！</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="911"/>
        <location filename="../src/ui/MainWindow.cpp" line="969"/>
        <location filename="../src/ui/MainWindow.cpp" line="973"/>
        <location filename="../src/ui/MainWindow.cpp" line="994"/>
        <location filename="../src/ui/MainWindow.cpp" line="996"/>
        <location filename="../src/ui/MainWindow.cpp" line="1096"/>
        <source>Btrfs Assistant</source>
        <translation>Btrfs 助手</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="911"/>
        <location filename="../src/ui/MainWindow.cpp" line="969"/>
        <location filename="../src/ui/MainWindow.cpp" line="1096"/>
        <source>Changes applied</source>
        <translation>更改已应用</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="946"/>
        <source>Are you sure you want to set read-only flag for %1?</source>
        <translation>确定要为 %1 设置只读标志吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="948"/>
        <source>Are you sure you want to clear read-only flag for %1?</source>
        <translation>确定要清除 %1 的只读标志吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="952"/>
        <source>Are you sure you want to set read-only flag for %1 subvolumes?</source>
        <translation>确定要为 %1 个子卷设置只读标志吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="954"/>
        <source>Are you sure you want to clear read-only flag for %1 subvolumes?</source>
        <translation>确定要清除 %1 个子卷的只读标志吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="564"/>
        <location filename="../src/ui/MainWindow.cpp" line="957"/>
        <location filename="../src/ui/MainWindow.cpp" line="1143"/>
        <location filename="../src/ui/MainWindow.cpp" line="1348"/>
        <location filename="../src/ui/MainWindow.cpp" line="1520"/>
        <source>Confirm</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="149"/>
        <location filename="../src/ui/MainWindow.cpp" line="173"/>
        <location filename="../src/ui/MainWindow.cpp" line="725"/>
        <location filename="../src/ui/MainWindow.cpp" line="748"/>
        <source>Stop</source>
        <translation>停止</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="158"/>
        <source>No balance running.</source>
        <translation>当前没有正在运行的均衡（Balance）操作。</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="646"/>
        <source>New Config</source>
        <translation>新建配置</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="646"/>
        <source>Cancel New Config</source>
        <translation>取消新建配置</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="974"/>
        <source>Failed to apply changes to the following subvolumes:</source>
        <translation>无法将更改应用到以下子卷：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="983"/>
        <source>Create &amp;snapshot...</source>
        <translation>创建快照(&amp;S)...</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="994"/>
        <source>Snapshot created</source>
        <translation>快照已创建</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1001"/>
        <source>Browse subvolume...</source>
        <translation>浏览子卷...</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1005"/>
        <source>Restore backup...</source>
        <translation>恢复备份...</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1010"/>
        <source>Set &amp;read-only flag</source>
        <translation>设置只读标志(&amp;R)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1016"/>
        <source>&amp;Clear read-only flag</source>
        <translation>清除只读标志(&amp;C)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1021"/>
        <source>&amp;Delete</source>
        <translation>删除(&amp;D)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1032"/>
        <source>Set cleanup algorithm to &amp;timeline</source>
        <translation>将清理算法设为时间线(&amp;T)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1035"/>
        <source>Set cleanup algorithm to &amp;number</source>
        <translation>将清理算法设为数量(&amp;N)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1038"/>
        <source>&amp;Remove cleanup algorithm</source>
        <translation>移除清理算法(&amp;R)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1041"/>
        <source>&amp;Delete snapshot</source>
        <translation>删除快照(&amp;D)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1044"/>
        <source>&amp;Change description</source>
        <translation>更改描述(&amp;C)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1138"/>
        <source>Please select a subvolume to delete first!</source>
        <translation>请先选择要删除的子卷！</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1143"/>
        <source>Are you sure you want to delete the selected subvolume(s)?</source>
        <translation>确定要删除所选子卷吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1161"/>
        <source>Snapper Snapshots Found</source>
        <translation>发现 Snapper 快照</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1162"/>
        <source>One or more of the selected subvolumes is a Snapper snapshot, would you like to remove the Snapper Metadata?(Recommended)</source>
        <translation>所选子卷中有一个或多个是 Snapper 快照，是否同时移除 Snapper 元数据？（推荐）</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1184"/>
        <source>Failed to delete subvolume!</source>
        <translation>删除子卷失败！</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1184"/>
        <source>Invalid subvolume ID</source>
        <translation>无效的子卷 ID</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1190"/>
        <source>You cannot delete mounted subvolume: </source>
        <translation>无法删除已挂载的子卷：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1191"/>
        <source>Please unmount the subvolume before deleting</source>
        <translation>请先卸载该子卷再删除</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1196"/>
        <source>Failed to delete subvolume </source>
        <translation>删除子卷失败：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1232"/>
        <location filename="../src/ui/MainWindow.cpp" line="1336"/>
        <location filename="../src/ui/MainWindow.cpp" line="1385"/>
        <location filename="../src/ui/MainWindow.cpp" line="1484"/>
        <location filename="../src/ui/MainWindow.cpp" line="1546"/>
        <source>Nothing selected!</source>
        <translation>未选择任何项目！</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1245"/>
        <source>Failed to restore snapshot</source>
        <translation>恢复快照失败</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1274"/>
        <source>Failed to find snapshot to browse</source>
        <translation>找不到要浏览的快照</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1300"/>
        <source>No config selected for snapshot</source>
        <translation>未为快照选择配置</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1306"/>
        <source>Enter a description for the snapshot</source>
        <translation>输入快照描述</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1306"/>
        <source>Description:</source>
        <translation>描述：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1348"/>
        <source>Are you sure you want to delete the selected snapshot(s)?</source>
        <translation>确定要删除所选快照吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1357"/>
        <source>Cannot delete snapshot</source>
        <translation>无法删除快照</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1409"/>
        <source>Change description</source>
        <translation>更改描述</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1410"/>
        <source>Changing &lt;u&gt;&lt;b&gt;%1&lt;/b&gt;&lt;/u&gt; snapshot(s) &lt;br&gt;&lt;br&gt;Enter a new description for the snapshot(s):</source>
        <translation>正在更改 &lt;u&gt;&lt;b&gt;%1&lt;/b&gt;&lt;/u&gt; 个快照&lt;br&gt;&lt;br&gt;请输入新的快照描述：</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1421"/>
        <source>Cannot change description of snapshot</source>
        <translation>无法更改快照描述</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1493"/>
        <source>Please select a single backup subvolume to restore!</source>
        <translation>请选择一个要恢复的备份子卷！</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1504"/>
        <source>The subvolume you selected is not a Btrfs Assistant backup</source>
        <translation>所选子卷不是 Btrfs 助手备份</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1515"/>
        <source>The subvolume is missing!</source>
        <translation>子卷不存在！</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1520"/>
        <source>Are you sure you want to restore the selected backup?</source>
        <translation>确定要恢复所选备份吗？</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1533"/>
        <source>Backup Restore</source>
        <translation>备份恢复</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1534"/>
        <source>Backup restoration complete.</source>
        <translation>备份恢复完成。</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1562"/>
        <source>Failed to set cleanup algorithm for snapshot %1</source>
        <translation>为快照%1 设置清理算法失败</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1581"/>
        <source>Disable Btrfs Quotas</source>
        <translation>禁用 Btrfs 配额</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="1583"/>
        <source>Enable Btrfs Quotas</source>
        <translation>启用 Btrfs 配额</translation>
    </message>
</context>
<context>
    <name>RestoreConfirmDialog</name>
    <message>
        <location filename="../src/ui/RestoreConfirmDialog.ui" line="14"/>
        <source>Dialog</source>
        <translation>对话框</translation>
    </message>
    <message>
        <location filename="../src/ui/RestoreConfirmDialog.ui" line="42"/>
        <source>Name for saved backup(Optional): </source>
        <translation>保存的备份名称（可选）：</translation>
    </message>
    <message>
        <location filename="../src/ui/RestoreConfirmDialog.ui" line="83"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="../src/ui/RestoreConfirmDialog.ui" line="90"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="../src/ui/RestoreConfirmDialog.ui" line="93"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
</context>
<context>
    <name>Snapper</name>
    <message>
        <location filename="../src/util/Snapper.cpp" line="359"/>
        <source>Failed to reset ownership of restored file</source>
        <translation>重置已恢复文件的所有权失败</translation>
    </message>
    <message>
        <location filename="../src/util/Snapper.cpp" line="388"/>
        <source>Failed to set config</source>
        <translation>设置配置失败</translation>
    </message>
</context>
<context>
    <name>SnapshotSubvolumeDialog</name>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.ui" line="14"/>
        <source>Create a snapshot</source>
        <translation>创建快照</translation>
    </message>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.ui" line="20"/>
        <source>Destination:</source>
        <translation>目标位置：</translation>
    </message>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.ui" line="29"/>
        <source>A filesystem path</source>
        <translation>文件系统路径</translation>
    </message>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.ui" line="42"/>
        <source>Browse...</source>
        <translation>浏览...</translation>
    </message>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.ui" line="51"/>
        <source>Read-only</source>
        <translation>只读</translation>
    </message>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.cpp" line="15"/>
        <source>Select a parent directory</source>
        <translation>选择父目录</translation>
    </message>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.cpp" line="24"/>
        <location filename="../src/ui/SnapshotSubvolumeDialog.cpp" line="30"/>
        <source>Btrfs Assistant</source>
        <translation>Btrfs 助手</translation>
    </message>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.cpp" line="24"/>
        <source>The destination path cannot be empty</source>
        <translation>目标路径不能为空</translation>
    </message>
    <message>
        <location filename="../src/ui/SnapshotSubvolumeDialog.cpp" line="31"/>
        <source>You entered a relative path. Do you want to continue with the resulting absolute path: %1?</source>
        <translation>输入的是相对路径。是否使用生成的绝对路径继续：%1？</translation>
    </message>
</context>
<context>
    <name>SubvolumeModel</name>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="16"/>
        <source>Parent ID</source>
        <translation>父 ID</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="18"/>
        <source>Subvol ID</source>
        <translation>子卷 ID</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="20"/>
        <source>Subvolume</source>
        <translation>子卷</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="22"/>
        <source>UUID</source>
        <translation>UUID</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="24"/>
        <source>Parent UUID</source>
        <translation>父 UUID</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="26"/>
        <source>Received UUID</source>
        <translation>接收 UUID</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="28"/>
        <source>Created</source>
        <translation>创建时间</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="30"/>
        <source>Generation</source>
        <translation>世代</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="32"/>
        <source>Read-only</source>
        <translation>只读</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="34"/>
        <source>Size</source>
        <translation>大小</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="36"/>
        <source>Filesystem</source>
        <translation>文件系统</translation>
    </message>
    <message>
        <location filename="../src/model/SubvolModel.cpp" line="38"/>
        <source>Exclusive</source>
        <translation>独占用量</translation>
    </message>
</context>
<context>
    <name>main</name>
    <message>
        <location filename="../src/main.cpp" line="15"/>
        <source>Btrfs Assistant</source>
        <translation>Btrfs 助手</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="22"/>
        <source>An application for managing Btrfs and Snapper</source>
        <translation>用于管理 Btrfs 和 Snapper 的应用程序</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="28"/>
        <source>List snapshots</source>
        <translation>列出快照</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="33"/>
        <source>Restore the given snapshot</source>
        <translation>恢复指定快照</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="34"/>
        <source>index of snapshot</source>
        <translation>快照索引</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="42"/>
        <source>Error: No Btrfs filesystems found</source>
        <translation>错误：未找到 Btrfs 文件系统</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="64"/>
        <source>Warning: No translations available</source>
        <translation>警告：没有可用的翻译</translation>
    </message>
</context>
</TS>
