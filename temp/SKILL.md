---
name: cms-smoking-test
description: 透過 CMS 版型測試控制台 API（CmsSmokingTestController）查詢測試站台設定、觸發「API 基礎測試 / 節點測試 / 冒煙測試」三層測試、輪詢報告並彙整結果。當使用者提到 "smoking test"、"冒煙測試"、"API 基礎測試"、"節點測試"，或要求對某個環境（如「新新併 sit」）跑測試時使用。
---

# CMS 版型測試控制台（CmsSmokingTest）

CMS 版型測試工具(原為 ToolHub 的 CmsSmokingTest,已獨立成單一服務)。三層測試共用同一組背景佇列與報告機制：

| 層級 | 名稱 | 執行端點 | 需要後台帳密 | 做什麼 |
|------|------|----------|--------------|--------|
| 第一層 | API 基礎測試 | `POST /api/api-test/execute` | 否 | 打後台／前台／預覽三個網域的固定 API 路徑清單 |
| 第二層 | 節點測試 | `POST /api/node-test/execute` | 否 | 讀各前台站台 `{url}/sitemap.json` 展開所有節點，逐一 HTTP GET |
| 第三層 | 冒煙測試 | `POST /api/cms-smoking-test/execute` | 是 | 開瀏覽器登入後台，爬遍節點依序截「頁面編輯／預覽／前台」三階段圖 |

三個執行端點都是**非阻塞**：立即回 `202 Accepted` + `{"uuid":"..."}`，實際由背景單執行緒佇列依序跑 Playwright。報告要另外輪詢。

- 頁面：`/cmsSmokingTest`（302 導向 `/CmsSmokingTest/index.html`）
- 原始碼：[CmsSmokingTestController.java](../../../java/tw/com/neux/smoketest/controller/CmsSmokingTestController.java)
- `{BASE_URL}`：正式站 `http://192.168.150.47`；本機開發 `http://localhost:8082`

---

## 1. Endpoint 規格

### 1.1 頁面

#### `GET /cmsSmokingTest` ／ `/cmsSmokingTest/` ／ `/cmsSmokingTest/index.html`
302 導向 `/CmsSmokingTest/index.html`（工具單頁）。

---

### 1.2 執行（三層測試 + 自動校稿）

#### `POST /api/cms-smoking-test/execute` — 冒煙測試（第三層）
建立執行紀錄丟入背景佇列，**不等待** Playwright 跑完。

Request body（`CmsSmokingTestRequest`）：

| 欄位 | 型別 | 必填 | 說明 |
|------|------|------|------|
| `backendDomain` | string | ✅ | 後台網域，不含 `/cms/login`（例：`http://192.168.0.171:8080`） |
| `backendAccount` | string | ✅ | 後台帳號 |
| `backendPassword` | string | ✅ | 後台密碼 |
| `testType` | string | ✅ | 固定帶 `"smoking-test"`，寫入 `cms_smoking_test.test_type` |
| `sites` | array | ✅ | 測試範圍，每筆對應一個「多網站」 |
| `sites[].name` | string | ✅ | 對應後台多網站清單顯示文字（如「官網」） |
| `sites[].url` | string | ✅ | 前台站台 URL（如 `http://192.168.0.171/portal/`） |
| `sites[].testNodeName` | string[] | ✅ | 空陣列＝測所有節點；非空＝只測列出的節點原始名稱 |
| `reporter` | string | ⬜ | 觸發人（如 Line 使用者姓名），寫進報告「觸發人」欄位；留空顯示「(未指定)」 |
| `testProject` | string | ⬜ | 測試專案名稱（如「新光人壽 SIT」），放在 LINE 通知訊息開頭 |
| `lineNotifyGroups` | string[] | ⬜ | 完成後要通知的 LINE 群組 ID；不帶＝不通知 |

```jsonc
// Request
{
  "backendDomain": "http://192.168.0.171:8080",
  "backendAccount": "shan",
  "backendPassword": "shan",
  "sites": [
    { "name": "官網", "url": "http://192.168.0.171/portal/", "testNodeName": ["首頁"] }
  ],
  "testType": "smoking-test"
}
// 202 Accepted
{ "uuid": "b6a8bea3-6313-4a1c-b64b-f49f3801d149" }
```

#### `POST /api/api-test/execute` — API 基礎測試（第一層）

Request body（`ApiTestExecuteRequest`）：

| 欄位 | 型別 | 必填 | 說明 |
|------|------|------|------|
| `adminDomain` | string | ✅ | 後台網域 |
| `frontendDomains` | string[] | ✅ | 前台網域清單，可多筆（多語系／多品牌共用同一套 CMS），每個都會跑一遍完整路徑清單 |
| `frontendDomain` | string | ⬜ | 舊版單筆寫法，僅為相容既有設定保留；與 `frontendDomains` 同時有值時以 `frontendDomains` 為準 |
| `previewDomain` | string | ✅ | 預覽網域 |
| `reporter` / `testProject` / `lineNotifyGroups` | — | ⬜ | 同上 |

```jsonc
// Request
{
  "adminDomain": "http://192.168.0.171:8080",
  "frontendDomains": ["http://192.168.0.171/portal"],
  "previewDomain": "http://192.168.0.171:4000"
}
// 202 Accepted
{ "uuid": "a558af78-3593-4521-83ff-c39ef10f9cd5" }
```

#### `POST /api/node-test/execute` — 節點測試（第二層）

Request body（`NodeTestExecuteRequest`）：

| 欄位 | 型別 | 必填 | 說明 |
|------|------|------|------|
| `sites` | array | ✅ | 每筆對應一個前台站台 |
| `sites[].name` | string | ✅ | 顯示名稱（前端未另外提供名稱時會與 `url` 相同） |
| `sites[].url` | string | ✅ | 前台站台 URL，`{url}/sitemap.json` 即節點來源 |
| `sites[].testNodeName` | string[] | ✅ | 空陣列＝檢查 sitemap 所有節點；非空＝只檢查列出的節點 |
| `reporter` / `testProject` / `lineNotifyGroups` | — | ⬜ | 同上 |

這一層**沒有後台帳密欄位**。

```jsonc
// Request
{
  "sites": [
    { "name": "http://192.168.0.171/portal", "url": "http://192.168.0.171/portal", "testNodeName": [] }
  ]
}
// 202 Accepted
{ "uuid": "a3b3d203-c616-4f5d-9f42-5e9206b2d9d8" }
```

#### `POST /api/cms-smoking-test/{uuid}/proofread` — 自動校稿
針對某一次**已完成的冒煙測試**，拿已截好的節點畫面與指定對照網址即時截圖做 AI 相似度比對，另外產一份報告（在執行歷程中以 `proofread` 層級出現）。回 `202` + **新的** uuid。

```jsonc
// Request（ProofreadRequest）
{
  "items": [
    { "site": "官網", "nodeName": "首頁", "compareUrl": "https://www.example.com/" }
  ]
}
```
- `site`：網站名稱，需對應那次冒煙測試的「網站名稱」（僅供報告顯示與辨識）
- `nodeName`：節點原始名稱，**必須與那次冒煙測試截圖時使用的名稱一致**，否則找不到對應截圖
- `compareUrl`：對照網址（通常是客戶現行正式站頁面）

---

### 1.3 執行歷程與報告

#### `GET /api/cms-smoking-test/list`
分頁列出執行歷程，依佇列時間新到舊排序。

Query：`page`（預設 `1`）、`limit`（預設 `10`）、`testType`（選填，留空列出所有層級）。

```jsonc
// 200 OK（ListDto）
{
  "items": [
    {
      "uuid": "b6a8bea3-...",
      "domain": "http://192.168.0.171:8080",
      "sites": "官網",
      "testNodeName": "首頁",
      "testType": "smoking-test",
      "status": "Finished",          // Waiting / Running / Finished / Failed / Deleted
      "enqueueTime": "2026-08-26T20:21:53.607588",
      "startTime": "2026-08-26T20:22:01.1",
      "endTime": "2026-08-26T20:35:40.2",
      "errorMessage": null,
      "reportAvailable": true
    }
  ],
  "page": 1, "limit": 10, "total": 1, "totalPages": 1
}
```

#### `GET /cmsSmokingTest/report/{uuid}` — 在瀏覽器開啟報告（HTML）
回報告 zip 內的 HTML 進入點（`report.html`，找不到退而找 `index.html`），已注入 `<base href="/cmsSmokingTest/report/{uuid}/">`；若該次執行含操作錄影（`recording.webm`），會在最上方補一列錄影連結。

| 狀態碼 | 意義 |
|--------|------|
| `200` | 報告已產生，body 為 HTML ← **輪詢時視為「拿到報告」** |
| `404` | 查無此 uuid |
| `409` | 尚未執行完成，或報告已被清除 ← **輪詢時代表「繼續等」** |

#### `GET /cmsSmokingTest/report/{uuid}/**` — 報告內的其他檔案
截圖／錄影等，供上面注入的 `<base>` 解析相對路徑後取用，以串流回傳（不把整包塞進同一份回應）。路徑即 zip 內的 entry 名稱（中文檔名會做 UTF-8 解碼）；查不到回 `404`。網址只多一個尾斜線（entry 為空）時 `302` 導回無斜線的報告頁。

#### `GET /api/cms-smoking-test/{uuid}/report` — 下載報告 zip
`status=Finished` 時可重複下載，回 `application/zip` + `Content-Disposition: attachment`。`404` 查無 uuid；`409` 尚未完成／報告已清除。

#### `DELETE /api/cms-smoking-test/{uuid}` — 軟刪除執行紀錄
`status` 改為 `Deleted` 並清除磁碟上的報告 zip。成功回 `204`；`404` 查無 uuid；`409` 未完成的紀錄無法刪除。

---

### 1.4 測試站台設定（`cms_smoking_test_site`）

一列＝一組可重複使用的環境設定（依 `name` 識別），`config` 內依測試層級分區段：`"api-test"` / `"node-test"` / `"smoking-test"`。

#### `GET /api/cms-smoking-test/site`
列出所有測試站台設定。Query：`testType`（選填，留空列出所有層級）。

```jsonc
// 200 OK（SiteConfigListDto）
{
  "items": [
    {
      "id": "a6d6902e-f360-4a94-8bfe-c0a55b70af68",
      "name": "neux_taishinlife_sit",
      "description": "新光人壽/新新併 sit",
      "testType": "smoking-test",
      "config": {
        "api-test": {
          "adminDomain": "http://192.168.0.171:8080",
          "frontendDomains": ["http://192.168.0.171/portal"],
          "previewDomain": "http://192.168.0.171:4000"
        },
        "node-test": {
          "frontendUrls": ["http://192.168.0.171/portal"]
        },
        "smoking-test": {
          "backendDomain": "http://192.168.0.171:8080",
          "backendAccount": "shan",
          "backendPassword": "shan",
          "sites": [
            { "name": "官網", "url": "http://192.168.0.171/portal/", "testNodeName": [] }
          ]
        }
      },
      "createdAt": "2026-08-26T20:21:53.607588",
      "updatedAt": "2026-08-26T22:23:09.7051861"
    }
  ]
}
```

#### `GET /api/cms-smoking-test/site/{id}`
取得單一設定。`400` id 非合法 UUID；`404` 查無。

#### `POST /api/cms-smoking-test/site` — 以 `name` 為鍵 upsert
`name` 不存在 → 新增，回 `201`；`name` 已存在 → **只覆蓋請求帶到的那些層級區段**（其他層級原封不動保留），回 `200`。

Request body（`CmsSmokingTestSiteRequest`）：`name`、`description`、`testType`、`config`（任意巢狀 JSON，依測試層級分 key，原樣存入 `config_json`）。

#### `PUT /api/cms-smoking-test/site/{id}`
更新指定設定。`400` id 非合法 UUID；`404` 查無 id；`409` 改名與既有 name 衝突。

#### `DELETE /api/cms-smoking-test/site/{id}`
硬刪除，回 `204`。`400` id 非合法 UUID；`404` 查無。

---

## 2. 觸發流程：使用者提到 "smoking test" / "冒煙測試"

當使用者提到 **"smoking test"** 或 **"冒煙測試"**（例如「smoking test 新新併 sit」）時，依下列步驟執行。全程用 `{BASE_URL}` 組出完整網址。

變數：`{SELECTED_ENV}`（使用者選定的那筆站台設定）、`{REPORT_RESULT}`（累積三層報告內容）。

### step 1 — 查測試清單並請使用者確認環境

呼叫 `GET {BASE_URL}/api/cms-smoking-test/site`，把回應中每筆的 `name`、`description` 與使用者提到的關鍵字比對相似度。

- **完全沒有相似的** → 直接回答 `查無相關測試設定`，結束對話。
- **有相似的** → 依下列格式回覆（把相似的每一筆完整 JSON 逐項列出、加上編號）：

```
請確認要測試的是環境
1. {
      "id": "a6d6902e-f360-4a94-8bfe-c0a55b70af68",
      "name": "neux_taishinlife_sit",
      "description": "新光人壽/新新併 sit",
      "testType": "smoking-test",
      "config": { ...完整 config... },
      "createdAt": "2026-08-26T20:21:53.607588",
      "updatedAt": "2026-08-26T22:23:09.7051861"
   }
2. …

請回覆 "小N smoking test neux_taishinlife_sit" 或 "小N smoking test 新新併 sit" 來確認要測試的環境
```

### step 2 — 確認使用者回覆

- 使用者回覆**不符合**上述任一環境 → 回答 `查無相關測試設定`，結束對話。
- 使用者回覆**符合**其中一筆 → 將該筆存為 `{SELECTED_ENV}`，繼續 step 3。

### step 3 — 判斷是否要跑 API 基礎測試

`{SELECTED_ENV}.config["api-test"]` 不存在 → 跳至 step 6；存在 → 進 step 4。

### step 4 — 觸發 API 基礎測試

把 `config["api-test"]` **原樣**當作 request body 呼叫 `POST {BASE_URL}/api/api-test/execute`：

```jsonc
// Request
{
  "adminDomain": "http://192.168.0.171:8080",
  "frontendDomains": ["http://192.168.0.171/portal"],
  "previewDomain": "http://192.168.0.171:4000"
}
// Response
{ "uuid": "a558af78-3593-4521-83ff-c39ef10f9cd5" }
```

記下 uuid 為 `{API_TEST_UUID}`。

### step 5 — 輪詢 API 基礎測試報告

每 **60 秒**呼叫一次 `GET {BASE_URL}/cmsSmokingTest/report/{API_TEST_UUID}`，直到回 `200`（`409` 代表還在跑，繼續等）。把拿到的報告內容併入 `{REPORT_RESULT}`，繼續 step 6。

### step 6 — 判斷是否要跑節點測試

`{SELECTED_ENV}.config["node-test"]` 不存在 → 跳至 step 9；存在 → 進 step 7。

### step 7 — 觸發節點測試

**注意欄位形狀不同**：設定裡存的是 `frontendUrls`（字串陣列），執行端點要的是 `sites`（物件陣列）。轉換規則為每個 URL 產一筆 `{ "name": url, "url": url, "testNodeName": [] }`。

```jsonc
// config["node-test"] = { "frontendUrls": ["http://192.168.0.171/portal"] }
// → POST {BASE_URL}/api/node-test/execute
{
  "sites": [
    { "name": "http://192.168.0.171/portal", "url": "http://192.168.0.171/portal", "testNodeName": [] }
  ]
}
// Response
{ "uuid": "a3b3d203-c616-4f5d-9f42-5e9206b2d9d8" }
```

記下 uuid 為 `{NODE_TEST_UUID}`。

### step 8 — 輪詢節點測試報告

每 **60 秒**呼叫一次 `GET {BASE_URL}/cmsSmokingTest/report/{NODE_TEST_UUID}`，直到回 `200`。報告併入 `{REPORT_RESULT}`，繼續 step 9。

### step 9 — 判斷是否要跑冒煙測試

`{SELECTED_ENV}.config["smoking-test"]` 不存在 → 跳至 step 12；存在 → 進 step 10。

### step 10 — 觸發冒煙測試

把 `config["smoking-test"]` 當作 request body，**額外補上 `"testType": "smoking-test"`**，呼叫 `POST {BASE_URL}/api/cms-smoking-test/execute`：

```jsonc
{
  "backendDomain": "http://192.168.0.171:8080",
  "backendAccount": "shan",
  "backendPassword": "shan",
  "sites": [
    { "name": "官網", "url": "http://192.168.0.171/portal/", "testNodeName": ["首頁"] }
  ],
  "testType": "smoking-test"
}
// Response
{ "uuid": "b6a8bea3-6313-4a1c-b64b-f49f3801d149" }
```

記下 uuid 為 `{SMOKING_TEST_UUID}`。

### step 11 — 輪詢冒煙測試報告

每 **60 秒**呼叫一次 `GET {BASE_URL}/cmsSmokingTest/report/{SMOKING_TEST_UUID}`，直到回 `200`。報告併入 `{REPORT_RESULT}`，繼續 step 12。

### step 12 — 分析並回報

分析 `{REPORT_RESULT}` 中的報告（失敗項目、錯誤訊息、異常節點），並在回傳訊息**最後**附上實際有跑的那幾層報告連結後結束對話：

```
1. API 基礎測試報告連結: {BASE_URL}/cmsSmokingTest/report/{API_TEST_UUID}
2. 節點測試報告連結: {BASE_URL}/cmsSmokingTest/report/{NODE_TEST_UUID}
3. 冒煙測試報告連結: {BASE_URL}/cmsSmokingTest/report/{SMOKING_TEST_UUID}
```

### 流程注意事項

- **三層依序執行**：背景是單執行緒佇列，前一層還在跑時後一層只會排隊，所以務必等前一層報告拿到（step 5 / 8 / 11）再送下一層。
- **某一層設定不存在就跳過**，不要自己補預設值；step 12 的連結清單只列實際有跑的層級。
- **輪詢判讀**：`200`＝完成、`409`＝還在跑或報告已清除、`404`＝uuid 不存在（此時應停止輪詢並回報錯誤）。若想順便看狀態與錯誤訊息，可查 `GET {BASE_URL}/api/cms-smoking-test/list`（`status` 為 `Failed` 時 `errorMessage` 有原因）。
- 冒煙測試會實際開瀏覽器爬完所有節點，數十分鐘是常態；輪詢要有耐心，不要提早判定失敗。
