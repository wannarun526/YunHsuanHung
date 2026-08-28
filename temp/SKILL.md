# SKILL：CMS 冒煙測試（ToolHub）

**觸發詞**：`smoking test`、`冒煙測試`

使用者提到觸發詞時，依「§3 冒煙測試流程」執行：
查測試站台設定 → 請使用者確認環境 → 依序跑 **API 基礎測試 → 節點測試 → 冒煙測試**，
每一層的報告都累積進 `{REPORT_RESULT}`，最後一次分析並回覆，附上所有報告連結。

> **三層是依序全部跑完，中間不因為某一層有失敗就停。** 某一層的設定不存在才會跳過那一層。

本檔案可經 HTTP 取得：`https://toolhub.neux.com.tw/CmsSmokingTest/SKILL.md`

---

## 1. 環境與變數

| 變數 | 說明 |
|---|---|
| `BASE_URL` | `https://toolhub.neux.com.tw`（正式站，結尾不加斜線） |
| `SELECTED` | 使用者在 step 2 選中的那一筆測試站台設定 |
| `{REPORT_RESULT}` | 報告累積器（空清單）。每跑完一層就把 `{ 層級, uuid, report.json 內容 }` 放進去，step 12 一次分析 |

> 對外網址**不含** `/toolhub` 前綴。WAR 在 WildFly 上的 context 是 `/toolhub`，但 Apache 已在反代層吃掉
> （`ProxyPass / http://localhost:8080/toolhub/`），加上前綴會 404。

---

## 2. Endpoint 規格

`CmsSmokingTestController` 的完整端點（共 15 個）。所有 `POST .../execute` 都是**非阻塞佇列**：
立即回 `202` + `uuid`，不等 Playwright 跑完。三個測試層級**共用同一個單執行緒佇列**與 `cms_smoking_test` 資料表，
同一時間只會有一個 Playwright 行程在跑（所以排隊時輪詢會比預期久）。

### 2.1 頁面

| Method | Path | 說明 |
|---|---|---|
| GET | `/cmsSmokingTest`<br>`/cmsSmokingTest/`<br>`/cmsSmokingTest/index.html` | 302 導向 `/CmsSmokingTest/index.html`（控制台單頁工具） |

### 2.2 執行（建立任務）

| Method | Path | 成功狀態 | 回應 |
|---|---|---|---|
| POST | `/api/api-test/execute` | 202 | `{ "uuid": "…" }` |
| POST | `/api/node-test/execute` | 202 | `{ "uuid": "…" }` |
| POST | `/api/cms-smoking-test/execute` | 202 | `{ "uuid": "…" }` |
| POST | `/api/cms-smoking-test/{uuid}/proofread` | 202 | `{ "uuid": "…" }` |

**POST /api/api-test/execute** — 第一層。純 HTTP 打後台／前台／預覽三區塊的固定 API 清單，不開瀏覽器，秒級完成。

```json
{
  "adminDomain": "http://192.168.0.171:8080",
  "frontendDomain": "http://192.168.0.171/portal",
  "previewDomain": "http://192.168.0.171:4000"
}
```

三個欄位皆必填，缺任一回 `400`（`缺少後台網域` / `缺少前台網域` / `缺少預覽網域`）。

**POST /api/node-test/execute** — 讀各站台前台的 `{url}/sitemap.json` 展開所有節點，
**用真的瀏覽器逐一開啟並等 render 完成**（一批 20 個分頁）。不需後台帳密。

```json
{
  "sites": [
    { "name": "http://192.168.0.171/portal", "url": "http://192.168.0.171/portal", "testNodeName": [] }
  ]
}
```

`sites` 不可為空、每筆 `url` 不可為空，否則 `400`。`testNodeName` 留空 = 測 sitemap 內所有節點。

**POST /api/cms-smoking-test/execute** — 第二層。開瀏覽器逐節點截「頁面編輯／預覽／前台」三階段圖並偵測頁面異常。

```json
{
  "backendDomain": "http://192.168.0.171:8080",
  "backendAccount": "shan",
  "backendPassword": "shan",
  "sites": [
    { "name": "官網", "url": "http://192.168.0.171/portal/", "testNodeName": [] }
  ],
  "testType": "smoking-test"
}
```

`testType` **必填**，只接受 `api-test` 或 `smoking-test`，其他值回 `400`。

**POST /api/cms-smoking-test/{uuid}/proofread** — 對某一次已完成的冒煙測試做 AI 校稿，
比對節點截圖與對照網址。`uuid` 須為已完成（`Finished`）且報告 zip 仍在的紀錄。

```json
{ "items": [ { "site": "官網", "nodeName": "公司治理", "compareUrl": "https://www.sinopac.com/governance.html" } ] }
```

#### `reporter`（選填，三支 execute 共用）

前三支 execute 都接受選填的 `reporter`，會寫進報告的「觸發人」欄位。
**建議一律帶入觸發這次測試的 LINE 使用者姓名**（群組訊息取發話者的 `displayName`，不是群組名稱）；
沒帶時報告的「觸發人」會留白，事後查不到是誰跑的。

```json
{ "adminDomain": "…", "frontendDomain": "…", "previewDomain": "…", "reporter": "Yun Shan" }
```

### 2.3 執行歷程

| Method | Path | 說明 |
|---|---|---|
| GET | `/api/cms-smoking-test/list?page=1&limit=10&testType=` | 分頁列出，依佇列時間新到舊 |
| DELETE | `/api/cms-smoking-test/{uuid}` | 軟刪除（status→`Deleted`，並清除 zip）；未完成的紀錄回 `409` |

`testType` 留空 = 全部；帶 `smoking-test` 時會一併列出由它衍生的 `proofread` 紀錄。

```json
{
  "items": [
    {
      "uuid": "…", "domain": "…", "sites": "…", "testNodeName": null,
      "testType": "api-test", "status": "Finished",
      "enqueueTime": "…", "startTime": "…", "endTime": "…",
      "errorMessage": null, "reportAvailable": true
    }
  ],
  "page": 1, "limit": 10, "total": 9, "totalPages": 1
}
```

`status` 值：`Enqueue` → `Executing` → `Finished` / `Failed`（或 `Deleted`）。

### 2.4 報告

| Method | Path | 說明 |
|---|---|---|
| GET | `/cmsSmokingTest/report/{uuid}` | **在瀏覽器直接開報告 HTML**（給人看、可分享，就是要附給使用者的連結） |
| GET | `/cmsSmokingTest/report/{uuid}/{檔名}` | 報告 zip 內的單一檔案（**給程式分析**，如 `report.json`） |
| GET | `/api/cms-smoking-test/{uuid}/report` | 下載整包報告 zip |

三者共用同一組狀態碼語意 —— **輪詢時務必分辨 409 與 404**：

| 狀態 | 意義 | 該怎麼處理 |
|---|---|---|
| `200` | 報告已就緒 | 取用 |
| `409` | **尚未執行完成**（`尚未執行完成,目前狀態: Enqueue/Executing`）或報告已被清除 | **繼續輪詢** |
| `404` | 找不到該 uuid 的紀錄 | **立刻中止**，不要再輪詢 |

zip 內含（依層級而異）：`report.html`、`report.json`、`console.log`、`process.log`，冒煙測試另有 `screenshots/`、`recording.webm`。

### 2.5 測試站台設定

| Method | Path | 說明 |
|---|---|---|
| GET | `/api/cms-smoking-test/site?testType=` | 列出設定 |
| GET | `/api/cms-smoking-test/site/{id}` | 取單筆（找不到 `404`） |
| POST | `/api/cms-smoking-test/site` | **以 `name` 為鍵 upsert**：新增回 `201`，已存在回 `200`（只覆蓋帶到的層級區段） |
| PUT | `/api/cms-smoking-test/site/{id}` | 整包更新（`404` / 改名衝突 `409`） |
| DELETE | `/api/cms-smoking-test/site/{id}` | 硬刪除，回 `204` |

**資料模型（重要）**：一個 `name` 一列，`config` 內以測試層級為 key 放各層級設定。
`testType` 欄位只代表「最後一次由哪個層級寫入」，**不可拿來判斷這筆支援哪些層級** —— 要看 `config` 有沒有對應的 key。

---

## 3. 冒煙測試流程

### step 1 — 查設定並請使用者確認環境

呼叫 `GET {BASE_URL}/api/cms-smoking-test/site`（**不帶** `testType`，要拿到全部）。

回應例：

```json
{
  "items": [
    {
      "id": "a6d6902e-f360-4a94-8bfe-c0a55b70af68",
      "name": "neux_taishinlife_sit",
      "description": "新光人壽/新新併 sit",
      "testType": "smoking-test",
      "config": {
        "api-test":     { "adminDomain": "http://192.168.0.171:8080",
                          "frontendDomain": "http://192.168.0.171/portal",
                          "previewDomain": "http://192.168.0.171:4000" },
        "node-test":    { "frontendUrls": ["http://192.168.0.171/portal"] },
        "smoking-test": { "backendDomain": "http://192.168.0.171:8080",
                          "backendAccount": "shan", "backendPassword": "shan",
                          "sites": [ { "name": "官網", "url": "http://192.168.0.171/portal/", "testNodeName": [] } ] }
      },
      "createdAt": "2026-08-26T20:21:53.607588",
      "updatedAt": "2026-08-26T22:23:09.7051861"
    }
  ]
}
```

把使用者提到的關鍵字（如「新新併sit」）與每筆的 `name`、`description` 做相似度比對。

- **完全沒有比對到相似的** → 回答 `查無相關測試設定`，**結束對話**。
- **有比對到相似的** → 依序編號列出候選（把整筆 JSON 原樣附上），並回覆：

```
請確認要測試的是環境
1. { …第 1 筆完整 JSON… }
2. { …第 2 筆完整 JSON… }
```

> 輸出前把 `config["smoking-test"].backendPassword` 遮蔽成 `***`，不要把後台密碼貼進聊天室。

### step 2 — 驗證使用者的選擇

使用者回覆的編號**不符合上述任一編號** → 回答 `查無相關測試設定`，**結束對話**。

符合則以該筆為 `SELECTED`，並建立空的 `{REPORT_RESULT}`。

### step 3 — 判斷是否跑第一層

`SELECTED.config["api-test"]` **不存在** → 跳 **step 6**；**存在** → 進 **step 4**。

### step 4 — 執行 API 基礎測試

把 `SELECTED.config["api-test"]` **原樣**當 request body：

```
POST {BASE_URL}/api/api-test/execute
{
  "adminDomain": "http://192.168.0.171:8080",
  "frontendDomain": "http://192.168.0.171/portal",
  "previewDomain": "http://192.168.0.171:4000"
}
```

回應：`{"uuid":"a558af78-3593-4521-83ff-c39ef10f9cd5"}` —— 記下這個 `uuid`（step 12 要用它組連結）。

### step 5 — 取得第一層報告

每 **60 秒**呼叫一次 `GET {BASE_URL}/cmsSmokingTest/report/{uuid}`，直到成功拿到（`200`）為止：

- `409` → 還在跑，繼續等
- `404` → 中止，回覆 `找不到執行紀錄 {uuid}`
- 超過 **30 次**（約 30 分鐘）→ 中止，回覆 `API 基礎測試逾時未完成`，並附上該層報告連結

拿到後，抓 `GET {BASE_URL}/cmsSmokingTest/report/{uuid}/report.json`（見 §4.1），
把 `{ 層級: "API 基礎測試", uuid, 報告內容 }` 存進 `{REPORT_RESULT}`，繼續 **step 6**。

> **不論這一層有沒有失敗都繼續往下跑**，分析統一留到 step 12。

### step 6 — 判斷是否跑節點測試

`SELECTED.config["node-test"]` **不存在** → 跳 **step 9**；**存在** → 進 **step 7**。

### step 7 — 執行節點測試

把 `config["node-test"].frontendUrls` 的每個 URL 展開成一筆 site（`name` 與 `url` 相同、`testNodeName` 為空陣列）：

```
POST {BASE_URL}/api/node-test/execute
{
  "sites": [
    { "name": "http://192.168.0.171/portal", "url": "http://192.168.0.171/portal", "testNodeName": [] }
  ]
}
```

回應：`{"uuid":"a3b3d203-c616-4f5d-9f42-5e9206b2d9d8"}` —— 記下 `uuid`。

### step 8 — 取得節點測試報告

輪詢方式同 **step 5**。拿到後抓 `report.json`（見 §4.2），
把 `{ 層級: "節點測試", uuid, 報告內容 }` 存進 `{REPORT_RESULT}`，繼續 **step 9**。

### step 9 — 判斷是否跑冒煙測試

`SELECTED.config["smoking-test"]` **不存在** → 跳 **step 12**；**存在** → 進 **step 10**。

### step 10 — 執行冒煙測試

把 `SELECTED.config["smoking-test"]` 原樣當 request body，**並補上 `testType`**（後端必填）：

```
POST {BASE_URL}/api/cms-smoking-test/execute
{
  "backendDomain": "http://192.168.0.171:8080",
  "backendAccount": "shan",
  "backendPassword": "shan",
  "sites": [
    { "name": "官網", "url": "http://192.168.0.171/portal/", "testNodeName": [] }
  ],
  "testType": "smoking-test"
}
```

回應：`{"uuid":"b6a8bea3-6313-4a1c-b64b-f49f3801d149"}` —— 記下 `uuid`。

> `sites[].testNodeName` **照設定裡的值原樣帶**，不要自行填入節點名稱。空陣列代表測該站台所有既有節點。

### step 11 — 取得冒煙測試報告

輪詢方式同 **step 5**，但**輪詢上限放寬**：冒煙測試要開瀏覽器逐節點截圖，
上百節點可能數十分鐘。拿到後抓 `report.json`（見 §4.3），
把 `{ 層級: "冒煙測試", uuid, 報告內容 }` 存進 `{REPORT_RESULT}`，繼續 **step 12**。

### step 12 — 分析並回覆，結束對話

把 `{REPORT_RESULT}` 內**所有**層級的報告一起分析（各層級的判讀方式見 §4），
先給人看得懂的結論，**再在訊息最後附上報告連結**：

```
1. API 基礎測試報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}, 2. 節點測試報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}, 3. 冒煙測試報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}
```

- **只列出實際有跑的層級**，並依序重新編號（例如只跑了節點測試與冒煙測試，就是 `1. 節點測試報告連結: … , 2. 冒煙測試報告連結: …`）。
- `{REPORT_RESULT}` 為空（三個層級都沒有設定）→ 回答 `此設定沒有任何可執行的測試層級`。

**結束對話。**

---

## 4. report.json 格式與判讀

**一律用 `report.json` 做分析，不要解析 report.html。** 取得方式：

```
GET {BASE_URL}/cmsSmokingTest/report/{uuid}/report.json
```

HTML 那支（`/cmsSmokingTest/report/{uuid}`）只用來當**給使用者點的連結**。

> `api-test` 與 `node-test` 的 report.html 表格**可點欄位標題排序**（再點一次反向），狀態欄升冪時失敗優先。

### 4.0 `meta`（三種報告共有）

```json
{
  "meta": {
    "reporter": "Yun Shan", "reportDate": "2026-08-28 14:30:12",
    "startedAt": "2026-08-28 14:29:45", "finishedAt": "2026-08-28 14:30:12",
    "durationMs": 27000, "duration": "27 秒"
  }
}
```

| 欄位 | 報告上的標題 |
|---|---|
| `reportDate` | 報告日期 |
| `startedAt` / `finishedAt` / `duration` | 測試期間 |
| `reporter` | 觸發人（沒帶 `reporter` 時為空字串） |

### 4.1 api-test

```json
{
  "meta": { … },
  "results": [
    { "block": "admin", "url": "/cms-api/SystemInfo", "fullUrl": "…", "status": 200, "ok": true, "elapsedMs": 120 },
    { "block": "frontend", "url": "/sitemap.json", "fullUrl": "…", "status": null, "ok": false,
      "elapsedMs": 15000, "error": "connect ECONNREFUSED" }
  ]
}
```

- `block`：`admin`（後台）／`frontend`（前台）／`preview`（預覽）
- `ok: false` 即為異常。`status: null` 代表連線層就失敗（連 HTTP 回應都沒收到），與收到 4xx/5xx 是兩種不同情況
- 回報寫法：`總計 {n} 筆 — 成功 {ok}、失敗 {fail}`；有失敗時列出異常的區塊中文名（後台／前台／預覽），同一區塊多筆只需列一次

### 4.2 node-test

```json
{
  "meta": { … }, "uuid": "…", "cfg": { … },
  "summaries": [
    { "name": "官網", "url": "http://…/portal/", "fetchUrl": "http://…/portal/sitemap.json",
      "fetchError": null, "total": 176, "ok": 156, "fail": 20, "skipped": 0 }
  ],
  "results": [
    { "site": "官網", "nodeName": "DemoL3-1", "kind": "content", "level": 5,
      "state": "fail", "url": "http://…/portal/1085", "status": 200,
      "finalUrl": "http://…/portal/error-page", "redirected": true,
      "networkErrors": ["500 http://…/portal/portal-api/Content/null"],
      "htmlLength": 41822, "elapsedMs": 4102,
      "failReasons": ["render 後被轉導至 http://…/portal/error-page",
                      "網路異常 1 筆(500 http://…/portal/portal-api/Content/null)"] }
  ]
}
```

**節點測試是用真的瀏覽器開啟每一頁並等 render 完成**，不是純 HTTP GET。失敗判定三條，任一成立即失敗：

| # | 判定 | 對應欄位 |
|---|---|---|
| 1 | 主文件 HTTP status 非 200 | `status` |
| 2 | render 後被轉導到別的頁（not-found / error-page） | `redirected` / `finalUrl` |
| 3 | 頁面載入時有子資源回 4xx/5xx 或請求失敗 | `networkErrors` |

- **回報失敗節點時直接引用 `failReasons`**，那是給人看的完整句子，不需要自己組
- 回報寫法：`總計 {total} 個節點 — 成功 {ok}、失敗 {fail}、略過 {skipped}`（多站台時跨站加總 `summaries`）
- `fetchError` 非 null 代表**整份 sitemap.json 沒抓到**（多半是前台 URL 填錯或站台掛了），該站台一個節點都沒測到，要特別提醒
- `state`：`ok` / `fail` / `skipped`（`skipped` = 該節點在前台沒有對應頁面，沒有開過瀏覽器，不計成敗）

### 4.3 smoking-test

```json
{
  "meta": { … },
  "fatalError": null,
  "results": [
    { "site": "官網", "phase": "edit", "nodeName": "首頁", "status": "ok",
      "screenshot": "screenshots/首頁-edit.png", "anomalies": ["壞圖: …"] },
    { "site": "官網", "phase": "web", "nodeName": "公司簡介", "status": "skipped",
      "reason": "此節點無前台連結" }
  ]
}
```

- `phase`：`edit`（頁面編輯）／`preview`（預覽）／`web`（前台）
- `status`：`ok` / `skipped`（遇異常或逾時跳過，值得關注）/ `na`（節點類型本身不支援，非異常）
- `anomalies`：頁面異常清單（壞圖、console 錯誤、未捕捉 JS 例外、資源請求失敗）。**與 `status` 各自獨立** —— 截圖成功的頁面照樣可能帶著一串異常
- 回報寫法：`總計 {總計} 筆 — 成功 {ok}、跳過 {skipped}、不適用 {na}、頁面異常 {anomaly}`
  - `總計` = `results.length`／`成功` = `status === "ok"`／`跳過` = `status === "skipped"`／`不適用` = `status === "na"`／`頁面異常` = `anomalies` 非空的**筆數**
- `fatalError` 非 null 代表測試中途整個中止，`results` 只是中止前收集到的部分，回報時要一併說明

---

## 5. 注意事項

- **三層依序全部跑完，不因某一層失敗就中止。** 只有「該層設定不存在」才跳過。分析統一在 step 12 做。
- **不要用 `testType` 欄位判斷一筆設定支援哪些層級**，一律看 `config` 有沒有對應的 key。
- **三個層級共用同一個單執行緒佇列。** 前一個任務還在跑時，新任務會排在後面等，輪詢時間會比預期長。
- **`POST .../execute` 回 202 不代表測試會成功**，只代表任務已入列。真正的成敗要看報告。
- **404 與 409 的差別很重要**：409 是「還在跑／報告被清了」要繼續等，404 是「這個 uuid 不存在」要立刻停。混在一起會無限輪詢。
- **step 12 的連結只列實際有跑的層級**，並依序重新編號。
- **設定裡含明文後台密碼**（`config["smoking-test"].backendPassword`）。step 1 列出候選時務必遮蔽成 `***`。
