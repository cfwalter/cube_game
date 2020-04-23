#ifndef QUAT_HPP
#define QUAT_HPP

struct Quat
{
    double r;
    double i;
    double j;
    double k;

    inline Quat operator*(Quat q)
    {
        // (ae-bf-cg-dh)+(af+be+ch-dg){i} +(ag-bh+ce+df){j} +(ah+bg-cf+de){k}
        float r = this->r*q.r - this->i*q.i - this->j*q.j - this->k*q.k;
        float i = this->r*q.i + this->i*q.r + this->j*q.k - this->k*q.j;
        float j = this->r*q.j - this->i*q.k + this->j*q.r + this->k*q.i;
        float k = this->r*q.k + this->i*q.j - this->j*q.i + this->k*q.r;
        return {r, i, j, k};
    };
};

#endif
