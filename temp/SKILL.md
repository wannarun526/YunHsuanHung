@src/main/resources/static/CmsSmokingTest/SKILL.md 調整 skill 如下


1. 將 @src/main/java/com/toolhubmaster/controller/CmsSmokingTestController.java 中所有 endpoint 規格列出

2. 當使用者提到 "smoking test" 或 "冒煙測試" 時觸發以下流程

ex:

回答使用者報告 {REPORT_RESULT}

step1. 使用者: smoking test 新新併sit => SKILL 觸發透過 GET /api/cms-smoking-test/site 取得目前測試清單，例如
responseBody = {
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
          "frontendUrls": [
            "http://192.168.0.171/portal"
          ]
        },
        "smoking-test": {
          "backendDomain": "http://192.168.0.171:8080",
          "backendAccount": "shan",
          "backendPassword": "shan",
          "sites": [
            {
              "name": "官網",
              "url": "http://192.168.0.171/portal/",
              "testNodeName": []
            }
          ]
        }
      },
      "createdAt": "2026-08-26T20:21:53.607588",
      "updatedAt": "2026-08-26T22:23:09.7051861"
    }
  ]
}

比對 responseBody 中哪幾個 name 或 description 跟使用者提到的最相似

若完全沒有比對到相似的直接回答 "查無相關測試設定" 並結束對話
若有比對到相似的回答 "
請確認要測試的是環境
1. {
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
          "frontendUrls": [
            "http://192.168.0.171/portal"
          ]
        },
        "smoking-test": {
          "backendDomain": "http://192.168.0.171:8080",
          "backendAccount": "shan",
          "backendPassword": "shan",
          "sites": [
            {
              "name": "官網",
              "url": "http://192.168.0.171/portal/",
              "testNodeName": []
            }
          ]
        }
      },
      "createdAt": "2026-08-26T20:21:53.607588",
      "updatedAt": "2026-08-26T22:23:09.7051861"
    }
2. …

請回覆 "小N smoking test neux_taishinlife_sit" 或 "小N smoking test 新新併 sit" 來確認要測試的環境
"

step2. 若使用者回覆不符合上述任一環境則直接回答 "查無相關測試設定" 並結束對話，若使用者回覆符合上述任一環境則將使用者回覆的編號存至 {SELECTED_ENV} 並繼續 step3

step3. 取用 {SELECTED_ENV} 的 config.api-test 不存在則跳至 step6，若該編號 config.api-test 存在則跳至 step4

step4. 若使用者回答符合上述編號且該編號有則拿該 config.api-test 塞入 request body call POST /api/api-test/execute，例如使用者回答 1 則 POST /api/api-test/execute 代入 reqest body 為
{
    "adminDomain": "http://192.168.0.171:8080",
    "frontendDomains": ["http://192.168.0.171/portal"],
    "previewDomain": "http://192.168.0.171:4000"
}

response body 為 {"uuid":"a558af78-3593-4521-83ff-c39ef10f9cd5"}

step5. 每 60 秒去 call 一次 GET /cmsSmokingTest/report/{uuid} 取得 step4 POST /api/api-test/execute 所產生的報告直到 GET /cmsSmokingTest/report/{uuid} 成功拿到為止，並將報告存至 {REPORT_RESULT} 繼續 step6

step6. 取用 {SELECTED_ENV} 的 config.node-test 不存在則跳至 step9，若該編號 config.node-test 存在則跳至 step7

step7. 拿使用者選取 config.node-test 塞入 request body call POST /api/node-test/execute，例如使用者回答 1 則 POST /api/node-test/execute 代入 reqest body 為
{
    "sites": [
        {
            "name": "http://192.168.0.171/portal",
            "url": "http://192.168.0.171/portal",
            "testNodeName": []
        }
    ]
}

response body 為 {"uuid":"a3b3d203-c616-4f5d-9f42-5e9206b2d9d8"}

step8. 每 60 秒去 call 一次 GET /cmsSmokingTest/report/{uuid} 取得 step7 POST /api/node-test/execute 所產生的報告直到 GET /cmsSmokingTest/report/{uuid} 成功拿到為止，並將報告存至 {REPORT_RESULT} 繼續 step9

step9. 取用 {SELECTED_ENV} 的 config.smoking-test 不存在則跳至 step12，若該編號 config.smoking-test 存在則跳至 step10

step10. 拿使用者選取 config.smoking-test 塞入 request body call POST /api/cms-smoking-test/execute，例如使用者回答 1 則 POST api/cms-smoking-test/execute 代入 reqest body 為
{
    "backendDomain": "http://192.168.0.171:8080",
    "backendAccount": "shan",
    "backendPassword": "shan",
    "sites": [
        {
            "name": "官網",
            "url": "http://192.168.0.171/portal/",
            "testNodeName": [
                "首頁"
            ]
        }
    ],
    "testType": "smoking-test"
}

response body 為 {"uuid":"b6a8bea3-6313-4a1c-b64b-f49f3801d149"}

step11. 每 60 秒去 call 一次 GET /cmsSmokingTest/report/{uuid} 取得 step10 POST /api/cms-smoking-test/execute 所產生的報告直到 GET /cmsSmokingTest/report/{uuid} 成功拿到為止，並將報告存至 {REPORT_RESULT} 繼續 step12

step12. 將 {REPORT_RESULT} 中報告進行分析，並在回傳訊息最後附上 "1. API 基礎測試報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}, 2. 節點測試報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}, 3. 冒煙測試報告連結: {BASE_URL}/cmsSmokingTest/report/{uuid}" 結束對話