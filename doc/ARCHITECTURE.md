```mermaid
graph TD
    %% Global Font & Scale Configuration
    
    classDef clientStyle fill:#06b6d4,stroke:#0891b2,color:#fff,stroke-width:2px;
    classDef gatewayStyle fill:#10b981,stroke:#059669,color:#fff,stroke-width:2px;
    classDef logicStyle fill:#8b5cf6,stroke:#7c3aed,color:#fff,stroke-width:2px;
    classDef extStyle fill:#f59e0b,stroke:#d97706,color:#fff,stroke-width:2px;
    classDef dbStyle fill:#6366f1,stroke:#4f46e5,color:#fff,stroke-width:2px;

    subgraph LayerA ["<span style='color:#1e3a8a; font-size:18px;'><b>A. Client Presentation Layer</b></span>"]
        User([🧑‍💻 Customer / Staff Role])
        SPA["🌐 Web Browser UI<br><i>Logic: Age >= 21 & Valid PAN</i>"]
    end

    subgraph LayerB ["<span style='color:#1e3a8a; font-size:18px;'><b>B. Core Backend Application Layer</b></span>"]
        CrowServer{{"🚪 C++ API Gateway<br><i>Logic: Security & HTTP Routing</i>"}}
        
        subgraph Business ["🧠 Business Logic Domain"]
            LoanFactory[["🏭 Loan Factory Pattern<br><i>Logic: Sorts Loan Subclasses</i>"]]
            RiskEngine[["⚖️ Risk & Scoring Engine<br><i>Logic: FOIR < 50% & Multipliers</i>"]]
        end
    end

    subgraph LayerC ["<span style='color:#1e3a8a; font-size:18px;'><b>C. External Cloud Microservice Layer</b></span>"]
        CibilService{{"💳 CIBIL Credit Bureau<br><code>cibil-mock-server.onrender.com</code><br><i>Logic: Fetches Live Score</i>"}}
    end

    subgraph LayerD ["<span style='color:#1e3a8a; font-size:18px;'><b>D. Dedicated DB Storage Layer</b></span>"]
        SQLite[(🗄️ SQLite DB<br>digi_loan_risk.db<br><i>Logic: SQL Persistence</i>)]
    end

    %% Flow Connections with Sequential Numbering (Clean Single-Line Labels)
    User --->|"<b>1. Clicks Submit (Req)</b>"| SPA
    SPA --->|"<b>2. Sends JSON Data (Req)</b>"| CrowServer
    
    CrowServer --->|"<b>3. Instantiates Domain</b>"| LoanFactory
    CrowServer --->|"<b>4. Outbound HTTPS Call</b>"| CibilService
    CibilService -.->|"<b>5. Returns Raw Score (Resp)</b>"| CrowServer
    
    CrowServer --->|"<b>6. Feeds Score & Financials</b>"| RiskEngine
    RiskEngine -.->|"<b>7. Returns Decision (Resp)</b>"| CrowServer
    
    CrowServer --->|"<b>8. Saves Record (Write)</b>"| SQLite
    SQLite -.->|"<b>9. Fetches History (Read)</b>"| CrowServer
    CrowServer -.->|"<b>10. Renders Receipt (Resp)</b>"| SPA

    %% Custom Link Styling (Solid Cyan = Request, Dotted Amber = Response)
    linkStyle 0,1,2,3,5,7 stroke:#06b6d4,stroke-width:3px;
    linkStyle 4,6,8,9 stroke:#f59e0b,stroke-width:3px,stroke-dasharray: 6 6;

    %% Apply Styles
    class User,SPA clientStyle;
    class CrowServer gatewayStyle;
    class LoanFactory,RiskEngine logicStyle;
    class CibilService extStyle;
    class SQLite dbStyle;