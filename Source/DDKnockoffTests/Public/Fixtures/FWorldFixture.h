#pragma once

class FWorldFixture {
public:
    explicit FWorldFixture(const FURL& URL = FURL());

    UWorld* GetWorld() const;

    ~FWorldFixture();

private:
    TWeakObjectPtr<UWorld> WeakWorld;
};
