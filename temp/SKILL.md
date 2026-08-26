# SKILL：CMS 冒煙測試（ToolHub）

**觸發詞**：`smoking test`、`冒煙測試`

使用者提到觸發詞時，依「§3 冒煙測試流程」逐步執行：先查測試站台設定 → 請使用者確認環境 → 依序跑 API 基礎測試 → 節點測試 → 冒煙測試，任一層出問題就回報並結束。

本檔案本身可經 HTTP 取得：`{BASE_URL}/CmsSmokingTest/SKILL.md`

---

## 1. 環境

| 變數 | 值 |
|---|---|
| `BASE_URL` | `https://toolhub.neux.com.tw`（正式站） |

回覆使用者的報告連結一律用 `{BASE_URL}/cmsSmokingTest/report/{uuid}`。

> 對外網址不含 `/toolhub` 前綴。WAR 在 WildFly 上的 context 是 `/toolhub`，但 Apache 已在反代層吃掉（`ProxyPass / http://localhost:8080/toolhub/`），加上前綴會 404。

---

## 2. Endpoint 規格

以下為 `CmsSmokingTestController` 的完整端點。所有 `POST .../execute` 都是**非阻塞佇列**：立即回 `202` + `uuid`，不等 Playwright 跑完；三個測試層級共用同一個單執行緒佇列與 `cms_smoking_test` 資料表，同一時間只會有一個 Playwright 行程在跑。

### 2.1 頁面

| Method | Path | 說明 |
|---|---|---|
| GET | `/cmsSmokingTest`<br>`/cmsSmokingTest/`<br>`/cmsSmokingTest/index.html` | 302 導向 `/CmsSmokingTest/index.html`（控制台單頁工具） |

### 2.2 執行（建立任務）

| Method | Path | 成功狀態 |
|---|---|---|
| POST | `/api/api-test/execute` | 202 |
| POST | `/api/node-test/execute` | 202 |
| POST | `/api/cms-smoking-test/execute` | 202 |
| POST | `/api/cms-smoking-test/{uuid}/proofread` | 202 |

四者的回應皆為：

```json
{ "uuid": "a558af78-3593-4521-83ff-c39ef10f9cd5" }
```

**POST /api/api-test/execute** — 第一層，純 HTTP 打後台／前台／預覽三區塊的固定 API 清單，不開瀏覽器，秒級完成。

```json
{
  "adminDomain": "http://192.168.0.171:8080",
  "frontendDomain": "http://192.168.0.171/portal",
  "previewDomain": "http://192.168.0.171:4000"
}
```

三個欄位皆必填，缺任一回 `400`（`缺少後台網域` / `缺少前台網域` / `缺少預覽網域`）。

**POST /api/node-test/execute** — 讀各站台前台的 `{url}/sitemap.json` 展開所有節點，逐一 HTTP GET。不需後台帳密。

```json
{
  "sites": [
    { "name": "http://192.168.0.171/portal", "url": "http://192.168.0.171/portal", "testNodeName": [] }
  ]
}
```

`sites` 不可為空、每筆 `url` 不可為空，否則 `400`。`testNodeName` 留空 = 測 sitemap 內所有節點。

**POST /api/cms-smoking-test/execute** — 第二層，開瀏覽器逐節點截「頁面編輯／預覽／前台」三階段圖並偵測頁面異常。

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

**POST /api/cms-smoking-test/{uuid}/proofread** — 對某一次已完成的冒煙測試做 AI 校稿，比對節點截圖與對照網址。

```json
{ "items": [ { "site": "官網", "nodeName": "公司治理", "compareUrl": "https://www.sinopac.com/governance.html" } ] }
```

`uuid` 須為已完成（`Finished`）且報告 zip 仍在的紀錄。

### 2.3 執行歷程

| Method | Path | 說明 |
|---|---|---|
| GET | `/api/cms-smoking-test/list?page=1&limit=10&testType=` | 分頁列出，依佇列時間新到舊 |
| DELETE | `/api/cms-smoking-test/{uuid}` | 軟刪除（status→`Deleted`，並清除 zip）；未完成的紀錄回 `409` |

`testType` 留空 = 全部；帶 `smoking-test` 時會一併列出由它衍生的 `proofread` 紀錄。

回應：

```json
{
  "items": [
    {
      "uuid": "…", "domain": "…", "sites": "…", "testNodeName": null,
      "testType": "api-test", "status": "Finished",
      "enqueueTime": "2026-08-26T21:03:11.12", "startTime": "…", "endTime": "…",
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
| GET | `/cmsSmokingTest/report/{uuid}` | **在瀏覽器直接開報告 HTML**（給人看、可分享） |
| GET | `/cmsSmokingTest/report/{uuid}/{檔名}` | 報告 zip 內的單一檔案（**給程式分析**，如 `report.json`） |
| GET | `/api/cms-smoking-test/{uuid}/report` | 下載整包報告 zip |

三者共用同一組狀態碼語意：

| 狀態 | 意義 | 該怎麼處理 |
|---|---|---|
| `200` | 報告已就緒 | 繼續分析 |
| `409` | **尚未執行完成**（`尚未執行完成,目前狀態: Enqueue/Executing`）或報告已被清除 | **繼續輪詢** |
| `404` | 找不到該 uuid 的紀錄 | **立刻中止**，不要再輪詢 |

zip 內含（依測試層級而異）：`report.html`、`report.json`、`console.log`、`process.log`，冒煙測試另有 `screenshots/`、`recording.webm`。

### 2.5 測試站台設定

| Method | Path | 說明 |
|---|---|---|
| GET | `/api/cms-smoking-test/site?testType=` | 列出設定 |
| GET | `/api/cms-smoking-test/site/{id}` | 取單筆（找不到 `404`） |
| POST | `/api/cms-smoking-test/site` | **以 `name` 為鍵 upsert**：新增回 `201`，已存在回 `200` |
| PUT | `/api/cms-smoking-test/site/{id}` | 整包更新（`404` / 改名衝突 `409`） |
| DELETE | `/api/cms-smoking-test/site/{id}` | 硬刪除，回 `204` |

**資料模型（重要）**：一個 `name` 一列，`config` 內以測試層級為 key 放各層級設定。`testType` 欄位只代表「最後一次由哪個層級寫入」，**不可拿來判斷這筆支援哪些層級** —— 要看 `config` 有沒有對應的 key。

`GET .../site?testType=xxx` 的篩選條件也是「`config` 內含該層級」，不是 `testType` 欄位。

回應：

```json
{
  "items": [
    {
      "id": "a6d6902e-f360-4a94-8bfe-c0a55b70af68",
      "name": "neux_taishinlife_sit",
      "description": "新光人壽/新新併 sit",
      "testType": "smoking-test",
      "config": {
        "api-test":     { "adminDomain": "…", "frontendDomain": "…", "previewDomain": "…" },
        "node-test":    { "frontendUrls": ["…"] },
        "smoking-test": { "backendDomain": "…", "backendAccount": "…", "backendPassword": "…",
                          "sites": [ { "name": "官網", "url": "…", "testNodeName": [] } ] }
      },
      "createdAt": "2026-08-26T20:21:53.607588",
      "updatedAt": "2026-08-26T22:23:09.7051861"
    }
  ]
}
```

---

## 3. 冒煙測試流程

### step 1 — 查設定並請使用者確認環境

呼叫 `GET {BASE_URL}/api/cms-smoking-test/site`（**不帶** `testType`，要拿到全部）。

把使用者提到的關鍵字（如「新新併sit」）與每筆的 `name`、`description` 做相似度比對。

- **完全沒有相似的** → 回答 `查無相關測試設定`，**結束對話**。
- **有相似的** → 依序編號列出候選（把整筆 JSON 原樣附上），並回覆：

```
請確認要測試的是環境
1. { …第 1 筆完整 JSON… }
2. { …第 2 筆完整 JSON… }
```

### step 2 — 驗證使用者的選擇

使用者的回答若不對應上述任一編號 → 回答 `查無相關測試設定`，**結束對話**。

以下用 `SELECTED` 代表選中那筆設定。**建立一個結果累積器 `RESULTS`（空清單）**，每跑完一層就把該層的摘要放進去（step 11 要用）。

### step 3 — 判斷是否跑第一層

`SELECTED.config["api-test"]` 不存在 → 跳 **step 7**；存在 → 進 **step 4**。

### step 4 — 執行 API 基礎測試

把 `SELECTED.config["api-test"]` **原樣**當 request body：

```
POST {BASE_URL}/api/api-test/execute
{ "adminDomain": "…", "frontendDomain": "…", "previewDomain": "…" }
```

取得回應的 `uuid`。

### step 5 — 輪詢報告

每 **60 秒**呼叫一次 `GET {BASE_URL}/cmsSmokingTest/report/{uuid}`，直到回 `200`。

- `409` → 還在跑，繼續等
- `404` → 中止，回覆 `找不到執行紀錄 {uuid}`
- 超過 **30 次**（約 30 分鐘）仍未完成 → 中止，回覆 `測試逾時未完成，報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}`

### step 6 — 分析第一層結果

抓 `GET {BASE_URL}/cmsSmokingTest/report/{uuid}/report.json`（見 §4.1）。

檢查後台／前台／預覽三區塊，找出 `ok !== true` 的項目。

- **有任一服務異常** → 回答並**結束對話**：

  ```
  服務 {失敗項目1}, {失敗項目2} 異常，報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}
  ```

- **全部正常** → 把摘要寫進 `RESULTS`，繼續 **step 7**。

### step 7 — 判斷是否跑節點測試

`SELECTED.config["node-test"]` 不存在 → 跳 **step 11**；存在 → 進 **step 8**。

### step 8 — 執行節點測試

把 `config["node-test"].frontendUrls` 每個 URL 展開成一筆 site（`name` 與 `url` 相同、`testNodeName` 為空陣列）：

```
POST {BASE_URL}/api/node-test/execute
{ "sites": [ { "name": "http://192.168.0.171/portal", "url": "http://192.168.0.171/portal", "testNodeName": [] } ] }
```

### step 9 — 輪詢報告

同 **step 5**（60 秒一次、409 續等、404 中止、上限 30 次）。

### step 10 — 分析節點測試結果

抓 `report.json`（見 §4.2）。

- **有失敗節點**（`state === "fail"`）→ 回答並**結束對話**：

  ```
  總計 {total} 個節點 — 成功 {ok}、失敗 {fail}、略過 {skipped}，報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}
  ```

- **沒有失敗節點** → 把摘要寫進 `RESULTS`，繼續 **step 11**。

### step 11 — 判斷是否跑第二層

`SELECTED.config["smoking-test"]` 不存在 → 把 `RESULTS` 內已累積的結果回報給使用者，**結束對話**。
（若 `RESULTS` 也是空的，代表這筆設定三個層級都沒有 → 回答 `此設定沒有任何可執行的測試層級`。）

存在 → 進 **step 12**。

### step 12 — 執行冒煙測試

把 `SELECTED.config["smoking-test"]` 原樣當 request body，**並補上 `testType`**（後端必填）：

```
POST {BASE_URL}/api/cms-smoking-test/execute
{
  "backendDomain": "…", "backendAccount": "…", "backendPassword": "…",
  "sites": [ { "name": "官網", "url": "…", "testNodeName": [] } ],
  "testType": "smoking-test"
}
```

> `sites[].testNodeName` 照設定裡的值原樣帶，不要自行填入節點名稱。空陣列代表測該站台所有既有節點。

### step 13 — 輪詢報告

同 **step 5**。冒煙測試要開瀏覽器逐節點截圖，耗時遠長於前兩層（上百節點可能數十分鐘），輪詢上限可視情況放寬。

### step 14 — 分析冒煙測試結果並回報

抓 `report.json`（見 §4.3），無論成功與否都回答並**結束對話**：

```
總計 {total} 筆 — 成功 {ok}、跳過 {skipped}、不適用 {na}、頁面異常 {anomaly}，報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}
```

---

## 4. report.json 格式

**一律用 `report.json` 做分析，不要解析 report.html。** 取得方式：

```
GET {BASE_URL}/cmsSmokingTest/report/{uuid}/report.json
```

HTML 那支（`/cmsSmokingTest/report/{uuid}`）只用來當**給使用者點的連結**。

### 4.1 api-test

```json
{
  "results": [
    { "block": "admin", "url": "/cms-api/SystemInfo", "fullUrl": "http://…/cms-api/SystemInfo",
      "status": 200, "ok": true, "elapsedMs": 120 },
    { "block": "frontend", "url": "/sitemap.json", "fullUrl": "…",
      "status": null, "ok": false, "elapsedMs": 15000, "error": "connect ECONNREFUSED" }
  ]
}
```

- `block`：`admin`（後台）／`frontend`（前台）／`preview`（預覽）
- `ok: false` 即為異常；`status: null` 代表連線層就失敗（連 HTTP 回應都沒收到），與收到 4xx/5xx 是兩種不同情況
- step 6 的「失敗項目」用 `block` 的中文名（後台／前台／預覽）表示，同一區塊多筆失敗只需列一次

### 4.2 node-test

```json
{
  "uuid": "…", "cfg": { … },
  "summaries": [
    { "name": "官網", "url": "http://…/portal/", "fetchUrl": "http://…/portal/sitemap.json",
      "fetchError": null, "total": 175, "ok": 170, "fail": 3, "skipped": 2 }
  ],
  "results": [
    { "site": "官網", "nodeName": "Home", "kind": "content", "level": 1,
      "state": "ok", "url": "…", "status": 200, "bytes": 5821, "elapsedMs": 88 }
  ]
}
```

- `state`：`ok` / `fail` / `skipped`（`skipped` = 該節點在前台沒有對應頁面，未發出請求，不計成敗）
- step 10 的三個數字直接取 `summaries` 加總即可（多站台時要跨站加總）
- `fetchError` 非 null 代表**整份 sitemap.json 沒抓到**（多半是前台 URL 填錯或站台掛了），此時該站台一個節點都沒測到，要特別提醒使用者

### 4.3 smoking-test

```json
{
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
- step 14 的四個數字：
  - `總計` = `results.length`
  - `成功` = `status === "ok"` 的筆數
  - `跳過` = `status === "skipped"` 的筆數
  - `不適用` = `status === "na"` 的筆數
  - `頁面異常` = `anomalies` 非空的**筆數**
- `fatalError` 非 null 代表測試中途整個中止，`results` 只是中止前收集到的部分，回報時要一併說明

---

## 5. 注意事項

- **不要用 `testType` 欄位判斷一筆設定支援哪些層級**，一律看 `config` 的 key。同一個 `name` 底下可同時有三個層級的區段。
- **三個層級共用同一個單執行緒佇列。** 前一個任務還在跑時，新任務會排在後面等，輪詢時間會比預期長。
- **`POST .../execute` 回 202 不代表測試會成功**，只代表任務已入列。真正的成敗要看報告。
- **404 與 409 的差別很重要**：409 是「還在跑／報告被清了」要繼續等，404 是「這個 uuid 不存在」要立刻停。把兩者混在一起會無限輪詢。
- **設定裡含明文後台密碼**（`config["smoking-test"].backendPassword`）。step 1 把整筆 JSON 列給使用者確認時，請把密碼遮蔽成 `***` 再輸出，不要把密碼貼進聊天室。
