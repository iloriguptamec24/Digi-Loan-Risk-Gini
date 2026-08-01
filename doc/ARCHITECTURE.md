# 🏗️ Technical Architecture & System Workflow (`ARCHITECTURE.md`)

This document outlines the system architecture, component design, data flow pipelines, design patterns, and database schema for the **Digi-Loan-Risk-Gini** microservice platform.

---

## 📐 System Architecture Overview

The system is built as a lightweight, multi-threaded C++ REST microservice using the **Crow HTTP framework** and **SQLite3**. It enforces clear operational separation between the user-facing web dashboard and backend risk evaluation logic.

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│                             Browser Frontend UI                             │
│                  (Role-Based Themed SSO: Guest / Staff)                     │
└──────────────────────────────────────┬──────────────────────────────────────┘
                                       │
                                       │ REST / HTTP (JSON + JWT)
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         Crow Web Server (src/main.cpp)                      │
│  ┌─────────────────────────┐                     ┌───────────────────────┐  │
│  │ SSO Auth Middleware     │                     │ Static File Server    │  │
│  │ (Mock JWT Inspector)    │                     │ (public/index.html)   │  │
│  └────────────┬────────────┘                     └───────────────────────┘  │
└───────────────┼─────────────────────────────────────────────────────────────┘
                │
                ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                          Core Business Logic Layer                          │
│                                                                             │
│  ┌────────────────────────┐                   ┌──────────────────────────┐  │
│  │   LoanFactory Class    │ ────────────────> │  CIBILScore Normalizer   │  │
│  │   (Creational Pattern) │                   │  (Encapsulated Domain)   │  │
│  └────────────┬───────────┘                   └──────────────────────────┘  │
│               │                                                             │
│               ▼                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                     Loan Base Abstract Interface                      │  │
│  │               (Polymorphic Credit Risk Evaluation)                    │  │
│  └──────┬─────────────┬──────────────┬──────────────┬─────────────┬──────┘  │
│         │             │              │              │             │         │
│         ▼             ▼              ▼              ▼             ▼         │
│    PersonalLoan  EducationLoan    AutoLoan       HomeLoan   BusinessLoan    │
└───────────────────────────────┬─────────────────────────────────────────────┘
                                │
                                │ C++ SQLite Wrapper
                                ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                             SQLite Database                                 │
│                      (data/digi_loan_risk.db)                               │
└─────────────────────────────────────────────────────────────────────────────┘
