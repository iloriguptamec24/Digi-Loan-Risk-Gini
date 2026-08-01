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