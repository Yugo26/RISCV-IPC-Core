#  RISC-V Microkernel IPC Core

##  專案背景與貢獻聲明
本專案為作業系統工程課程之延伸實作。微核心的基礎底層架構（包含 Bootloader、Task 排程器、硬體中斷初始化）為指導教授提供之開源框架。
**我的核心貢獻在於：** 深入研讀並理解該 OS 框架後，從零實作並無縫整合了高可靠度的 IPC 與任務同步機制，並基於 RISC-V 的架構完成了 User-to-Kernel 的 System Call 封裝，並成功測試了微核心的可用性與併發安全性。

File Guide 以下為由我主力開發的核心檔案：
*   `Mutex.c` / `ipc.h`：具備遞迴機制、防死鎖與防 Lock Stealing 的互斥鎖。
*   `msgq.c`：基於 Ring Buffer 實作的 O(1) 雙向阻塞訊息佇列。
*   `Event.c`：支援 AND/OR 邏輯與廣播機制的事件標誌組。
*   `syscall.c` (部分修改)：基於 RISC-V ABI 暫存器傳參規則的系統呼叫封裝。

---
##  Key Features

### 1. Mutex : 遞迴機制以及防禦虛假喚醒
*   Recursive Locking：在 MutexCB 中引入 `owner` 與 `lockCount`，允許同一 Task 多次獲取同一把鎖而不發生 Self-Deadlock。
*   Lock Stealing Prevention：在 Task 因等待鎖而被喚醒時，實作二次狀態確認機制。
*   Safe Timeout：修復了 Task 超時醒來後未從等待清單移除的致命 Bug，透過嚴格的 `list_remove` 確保 Linked List 指標完整性。

### 2. Message Queue: O(1) Ring Buffer and Deep Copy
*   Two-way Blocking：實作 Producer-Consumer 模型，滿載時阻塞發送者，空載時阻塞接收者，並在狀態改變時觸發 `task_yield()` 進行搶佔式排程。
*   Circular Buffer：利用雙指標 (head/tail) 機制，取代傳統陣列的資料搬移，達成時間複雜度 O(1) 的高效讀寫。
*   Deep Copy：強制使用 `memcpy` 將資料從 User Space 完整複製至 Kernel 內部。

### 3. Event Group: 位元運算與廣播喚醒
*   Multi-condition Sync：精準運用 Bitwise 操作，支援 `EVENT_WAIT_AND` 與 `EVENT_WAIT_OR` 邏輯，並支援條件達成後 Auto-Clear。
*   Broadcast & Safe Iteration：事件觸發時掃描 Wait List 喚醒所有符合條件的 Task。實作 Safe Iteration，在移除節點前預存 `next` 指標，完美避開 Use-after-free 記憶體崩潰問題。

## Tech Stack
*   架構與核心: RISC-V Architecture, Preemptive Scheduling, Context Switch, System Call (ecall)
*   IPC : Mutex, Message Queue (Ring Buffer), Event Group, Semaphore
*   程式語言: C (Pointer arithmetic, Bitwise operations, Memory Management)
