# 🏦 Digi-Loan-Risk-Gini

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Crow Framework](https://img.shields.io/badge/Framework-Crow%20v1.0-brightgreen.svg)
![Database](https://img.shields.io/badge/Database-SQLite3-orange.svg)
![License](https://img.shields.io/badge/License-MIT-blue)

A multi-tiered, asynchronous C++ microservice designed for automated banking credit risk assessment, Loan-to-Income (LTI) ratio scoring, and Fixed Obligations to Income Ratio (FOIR) calculations. Built with **Crow**, **SQLite3**, and modern Object-Oriented Design Patterns.

---

## 📌 Project Overview

**Digi-Loan-Risk-Gini** provides real-time credit decisioning for personal and commercial banking loans. It features a responsive, eye-comfortable web frontend supporting multi-role Single Sign-On (SSO) simulation (**Guest**, **Underwriter**, and **Admin**).

> [!NOTE]
> **Key Architecture Highlight:** Uses a **Factory Pattern** and dynamic runtime polymorphism to evaluate distinct loan categories with custom risk multipliers.

---

## 🏗️ Object-Oriented Architecture (OOP)

This project leverages fundamental Object-Oriented Programming principles to ensure scalability and clean separation of concerns:

| OOP Concept | File Location | Application in Project |
| :--- | :--- | :--- |
| **Abstraction** | `include/database.hpp` | The `Loan` abstract class defines a pure virtual function `evaluate() = 0` to enforce evaluation logic on all loan types. |
| **Encapsulation** | `include/cibil_score.hpp` | `CIBILScore` hides `rawScore` internally while exposing clean methods like `isSubprime()` and `getNormalizedSubScore()`. |
| **Inheritance** | `src/database.cpp` | `PersonalLoan`, `EducationLoan`, `AutoLoan`, `HomeLoan`, and `SmallBusinessLoan` derive from the base `Loan` class. |
| **Polymorphism** | `src/database.cpp` | Each loan class overrides `evaluate()` to apply category-specific risk multipliers (e.g., `0.85x` for Home vs `1.35x` for Personal). |
| **Factory Pattern** | `include/database.hpp` | `LoanFactory::createLoan()` dynamically instantiates `std::unique_ptr<Loan>` based on user input. |

---

## 🏛️ Detailed System Architecture Breakdown

### 1. Client Presentation Layer (Frontend)
* **Technology:** Tailwind CSS + Vanilla JavaScript Single Page Application (SPA).
* **Role & Function:** Serves as the user-facing interface for customers, underwriters, and administrators. Enforces strict pre-submission validations (such as checking applicant age rules and verifying 10-character PAN formats) directly in the browser to prevent invalid payloads from hitting the server.

### 2. Core Backend Application Layer
* **Technology:** C++ utilizing the **Crow Web Framework** (hosted locally or on Render via Port 18080).
* **Role & Function:** Acts as the primary API Gateway and routing engine. Handles incoming REST payloads, validates JWT security tokens, manages CORS policies, and orchestrates the entire loan evaluation pipeline.

### 3. Business Logic Domain
* **Technology:** Object-Oriented **Loan Factory Pattern** combined with a custom **Risk Scoring Engine**.
* **Role & Function:** Dynamically instantiates polymorphic loan subclasses and executes complex financial algorithms, including evaluating FOIR, LTI ratios, loan type weightable factors, and collateral-backed risk multipliers.

### 4. External Cloud Microservice Layer
* **Technology:** Third-Party CIBIL Mock Microservice hosted on Render (`https://cibil-mock-server.onrender.com/api/v1/cibil/score`).
* **Role & Function:** Decoupled external web service simulating real-world banking infrastructure, fetching live raw credit scores securely over HTTPS before final risk calculation.

### 5. Dedicated Database Storage Layer
* **Technology:** Embedded SQLite Database (`data/digi_loan_risk.db`).
* **Role & Function:** Provides ACID-compliant SQL persistence. Stores all approved, rejected, and pending loan applications, tracks user history, and supports administrative overrides or database purges.

---

## ⚖️ Architectural Trade-Off Analysis & Technology Comparison

| Layer / Component | Chosen Technology | Alternative Considered | Rationale & Trade-off Analysis |
| :--- | :--- | :--- | :--- |
| **Frontend UI** | **Vanilla JS + Tailwind CSS** | React / Angular SPA | **Performance & Simplicity:** Eliminates heavy build-tool pipelines and client-side framework bloat while delivering a clean, responsive interface instantly. |
| **Backend Framework** | **C++ with Crow Web Framework** | Node.js (Express) / Python (FastAPI) | **Raw Execution Speed:** C++ provides near-metal performance for heavy financial algorithms and multi-threaded routing in a compiled language. |
| **Credit Bureau Integration** | **Render-Hosted Cloud Mock API** | Local In-Memory Mock Function | **Realistic Architecture:** Decoupling the CIBIL service to an external cloud instance mimics genuine production enterprise banking environments and network latency. |
| **Database Storage** | **Embedded SQLite (`.db`)** | PostgreSQL / MySQL Server | **Portability & Zero-Config:** SQLite offers full SQL capabilities and ACID compliance inside a single portable file, removing external database daemon overhead. |

---

## 🧮 Comprehensive Business Logic & Underwriting Engine

### 1. Fixed Obligation to Income Ratio (FOIR) & Disposable Income
* **Core Rule:** Total household and individual debt servicing must not exceed **50%** of net monthly earnings.
* **Calculation Formula:**
  $$\text{FOIR} = \frac{\text{Existing Monthly EMIs} + \text{Proposed Loan EMI}}{\text{Net Monthly Income}} \times 100$$
* **Disposable Income Buffer (DDI):** 
  $$\text{DDI} = \text{Net Monthly Income} - (\text{Existing EMIs} + \text{Proposed EMI} + \text{Estimated Living Expenses})$$

### 2. Loan-to-Income (LTI) & Leverage Caps
* **Home Loans:** Maximum LTI cap of **$5.0x$** annual gross income.
* **Auto Loans:** Maximum LTI cap of **$1.5x$** annual gross income.
* **Personal Loans:** Maximum LTI cap of **$2.0x$** annual gross income.

### 3. Dynamic Loan Type Weightable Factors ($W_L$)
* **Home Loan ($W_L = 0.85x$):** Backed by physical real estate collateral (Low Risk).
* **Auto Loan ($W_L = 1.00x$):** Backed by a depreciating vehicular asset (Standard Risk).
* **Personal Loan ($W_L = 1.25x$):** Unsecured exposure requiring higher credit buffering (Higher Risk).
* **Education / Business Loan ($W_L = 1.10x$):** Semi-secured growth capital (Moderate Risk).

### 4. Credit Bureau Thresholds & Scoring Equation
* **Secured Products:** Minimum floor score of **620**.
* **Unsecured Products:** Minimum floor score of **650**.
* **Composite Risk Index Equation:**
  $$\text{Composite Risk Index} = \left(\frac{\text{Requested Amount}}{\text{Annual Income}} \times W_L\right) + \text{FOIR Weight} - \left(\frac{\text{CIBIL Score}}{200}\right)$$

---

## 📊 Automated Decision-Making & Override Matrix

| Rule / Condition | Threshold Criteria | Automated Engine Verdict | Underwriter / Admin Override Conditions |
| :--- | :--- | :--- | :--- |
| **Clean Compliance** | FOIR $< 50\%$, LTI within limit, CIBIL $\ge$ threshold | **APPROVED** | N/A (Standard automated pass flow) |
| **Borderline FOIR** | FOIR between $50\% - 55\%$ with CIBIL $> 750$ | **REJECTED** (Automated) | **Overridable:** Can be overridden to **Approved** if verified proof of liquid secondary assets or co-applicant backing is provided. |
| **Low Credit Score** | CIBIL score falls slightly below threshold ($600 - 649$) | **REJECTED** (Automated) | **Overridable:** Can be overridden to **Approved** if the borrower offers additional collateral security or a higher down payment. |
| **High LTI / Debt** | FOIR $> 55\%$ or LTI exceeds max multiplier cap | **HARD REJECT** | **Non-Overridable:** Strict regulatory risk limits prohibit manual overrides for extreme over-leveraging. |

---

## 🎨 SSO Roles & Eye-Comfortable UI Themes

| Role | Theme Palette | Visibility / Features |
| :--- | :--- | :--- |
| 🟢 **Guest** | **Sage Mint** (`#f0fdf4`) | Public applicant entry form (PAN, Mobile, DOB, Email, Consent) & real-time identity validation check. |
| 🔵 **Underwriter** | **Sky Blue** (`#f0f9ff`) | Direct CIBIL score input + full access to live risk evaluation reports & score metrics table. |
| 🟣 **Admin** | **Soft Lavender** (`#faf5ff`) | Full underwriting view + privilege to manually **Override** automated system decisions. |

---

## 🚀 Getting Started

### Prerequisites

* **C++ Compiler:** `g++` or `clang` with C++17 support
* **Build System:** CMake (v3.14+)
* **Dependencies:** `SQLite3`, `OpenSSL`, and `libasio-dev`

On Ubuntu / Debian / GitHub Codespaces, install dependencies via:

```bash
sudo apt-get update && sudo apt-get install -y libasio-dev libsqlite3-dev libssl-dev