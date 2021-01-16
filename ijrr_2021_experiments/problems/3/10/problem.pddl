(define (problem ijrr_10)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street17 street42 - street
		building11 building13 building25 building30 - building
	)
	(:init
		(atLocation survivor0 street17)
		(atLocation survivor1 building25)
		(onFire building11)
		(onFire building30)
		(underRubble street42)
		(underRubble building13)
		(needsRepair building11)
		(needsRepair building13)
		(needsRepair building30)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street42))
		(not (needsRepair building11))
		(not (needsRepair building13))
		(not (needsRepair building30))
	))
	(:metric minimize total-time)
)
