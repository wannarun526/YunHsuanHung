# Hung Yun Hsuan（Sean）

**Software Engineer · Fintech and Payments**

yunshanghong@gmail.com · +886 918-990-5266 · Taiwan (open to relocation) · [github.com/wannarun526](https://github.com/wannarun526)

---

## Summary

Software engineer with 6 years of production experience across banking, insurance, and digital-payment systems, now working as an independent studio who owns delivery end to end — requirements, scoping, estimation, implementation, and launch. Ships across the full stack: React Native apps on iOS and Android, the NestJS and .NET services behind them, and the Kubernetes and GCP infrastructure they run on. Built off-chain payment flows for XSGD, Singapore's largest stablecoin issuer, on a platform serving 100K+ users and 100K+ daily transactions, and architected a customer-facing AI assistant on Vertex AI Agent Engine. Stays on the hook once features are live — diagnosing race conditions, deadlocks, and N+1 bottlenecks under real load — and ran internal engineering training across five repositories.

## Technical Skills

| | |
|---|---|
| **Performance & Reliability** | Concurrency control (pessimistic locking, deadlock resolution), query optimisation and index design, caching strategy, memory/OOM debugging, load balancing, production monitoring and incident diagnosis |
| **Backend** | C# / .NET, Java (Maven), NestJS (Node.js), FastAPI (Python), REST API design, three-tier architecture, CI/CD |
| **Data** | SQL Server, MySQL, Redis |
| **Cloud & Infrastructure** | GCP (primary), Kubernetes, Docker, Vertex AI, Azure, Azure Pipelines, Git |
| **Domain** | Retail banking, life insurance, stablecoin issuance and off-chain payments, high-volume loyalty platforms |
| **Frontend & Mobile** | Angular, React, Vue, React Native (iOS/Android) |

## Professional Experience

### Software Engineer
*Jan 2023 – Present · Remote*

Sole proprietorship. Retained by enterprise and financial-services clients to diagnose and remediate production performance failures, and to own backend delivery end to end — requirements, scoping, estimation, implementation, and launch. Four principal engagements below.

**Digiwin — DigiKnow** *(ERP vendor; 100K+ member platform)*

- Eliminated data inconsistency in a points-redemption feature by introducing pessimistic concurrency control, resolving **race conditions and database deadlocks** caused by heavy contention on shared inventory records under concurrent redemption.
- Cut timing-out APIs to **sub-second responses** by diagnosing **N+1 query patterns**, restructuring data access behind a reusable caching layer, adding missing **database indexes**, and simplifying over-joined queries.
- Eliminated **out-of-memory failures** in large administrative data exports by converting single-pass extraction to batched, paginated processing.
- Introduced **Kubernetes load balancing** and GCP Cloud Logging for production monitoring and incident diagnosis, improving reliability and horizontal scalability.
- Delivered the member-facing mobile app for **iOS and Android from a single React Native codebase**, extending the platform to mobile without a second native team.
- Architected a customer-facing AI assistant on **Vertex AI Agent Engine** using a tool/skill routing pattern, replacing multi-call orchestration with agent-driven tool selection and reducing third-party LLM dependency.

**Sakura Taiwan** *(consumer appliance manufacturer; public corporate site)*

- Led a **cross-functional investigation** with infrastructure and DBA teams to diagnose and resolve slow page loads on the production site, tracing the bottleneck across application, database, and infrastructure layers.
- Applied the same query and caching remediation to bring timing-out endpoints under one second.

**Taishin Life Insurance** *(life insurer; production policy platform)*

- Modernised a live insurance platform across six major framework versions (**Angular 11 → 17**), clearing accumulated dependency debt and improving development throughput by over 80%.
- Maintained and enhanced the **Java / Maven** CMS admin backend behind the platform, delivering feature changes and defect fixes on the live system.
- Partnered with the QA function to prototype an AI-assisted automated testing tool.

**Cross-engagement**

- Owned client-facing delivery independently across all engagements, and ran internal engineering training across five repositories.

### Full-Stack Engineer — Xfers / StraitsX / Fazz
*Apr 2022 – Jan 2023 · Remote*

- Built off-chain payment-flow logic and user-facing interfaces for **XSGD, Singapore's largest stablecoin issuer**, on a platform serving **100K+ users and 100K+ daily transactions**.
- Optimised internal operations tooling, reducing the operations team's daily manual workload.
- Integrated **MetaMask** wallet connectivity, improving user onboarding conversion.

### Software Engineer — Provision Information
*Nov 2020 – Apr 2022 · Taipei, Taiwan*

- Delivered external-facing retail banking systems for **E.SUN Bank**.
- Unblocked parallel development across the team by defining complete interface contracts backed by **mock services**.
- Automated releases via **CI/CD** (Azure Pipelines), driving the deployment error rate to **zero**.
- Led development of a navigation-verification module enforcing that users complete required, validated steps in sequence — a regulatory requirement for the banking flow.

### Software Engineer — TPI Software
*Mar 2020 – Nov 2020 · Taipei, Taiwan*

- First engineering role; delivered full-stack features for banking systems under senior mentorship.

### Civil Engineer — T.Y. Lin International Group
*2016 – 2018 · Taipei, Taiwan*

- Structural analysis and design optimisation. Transitioned to software engineering in 2018.

## Education

**B.S. in Civil Engineering** — National Cheng Kung University (NCKU), Taiwan · *2012 – 2016*
